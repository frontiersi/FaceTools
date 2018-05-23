/************************************************************************
 * Copyright (C) 2017 Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#include <ActionCloseFaceModels.h>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
using FaceTools::Action::ActionCloseFaceModels;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::CloseFaceModelsHelper;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


ActionCloseFaceModels::ActionCloseFaceModels( const QString& dname, const QIcon& ico, const QKeySequence& keys, CloseFaceModelsHelper* chelper)
    : FaceAction( dname, ico, keys, true/*this action disabled on other actions executing*/),
      _chelper(chelper)
{
}   // end ctor


bool ActionCloseFaceModels::doBeforeAction( FaceControlSet& fset)
{
    FaceModelSet fms = fset.models();
    for ( FaceModel* fm : fms)
    {
        // If the FaceModel hasn't been saved and the user doesn't want to close it (after prompting), remove from the action set.
        bool doclose = false;
        if ( _chelper->isSaved(fm))
            doclose = true;
        else
        {
            std::string fname = boost::filesystem::path( _chelper->filepath(fm)).filename().string();
            if ( QMessageBox::Yes == QMessageBox::question( _chelper->parentWidget(),
                                                tr(("Unsaved changes on \"" + fname + "\"").c_str()),
                                                tr("Model has unsaved changes! Close without saving anyway?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
            {
                doclose = true;
            }   // end if
        }   // end if

        if ( doclose)
            _cset.insert(fm);
    }   // end for
    return !_cset.empty();
}   // end doBeforeAction


bool ActionCloseFaceModels::doAction( FaceControlSet& rset)
{
    // Remove all entries in rset that will be closed (nominally all of them)
    std::for_each(std::begin(_cset), std::end(_cset), [&](auto fm){rset.erase(fm);});
    _chelper->close( _cset);
    _cset.clear();
    return true;
}   // end doAction
