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
using FaceTools::Action::ActionCloseFaceModels;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::CloseFaceModelsHelper;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


ActionCloseFaceModels::ActionCloseFaceModels( CloseFaceModelsHelper* chelper)
    : FaceAction(true/*this action disabled on other actions executing*/),
      _chelper(chelper), _icon( ":/icons/CLOSE"), _scut( Qt::CTRL + Qt::Key_W)
{
    setAsync(true);
}   // end ctor


bool ActionCloseFaceModels::doBeforeAction( FaceControlSet& fset)
{
    for ( FaceControl* fc : fset)
    {
        FaceModel* fm = fc->data();
        _cset.insert(fm);
        // If the FaceModel hasn't been saved and the user doesn't want to close it (after prompting), remove from the action set.
        if ( !_chelper->isSaved(fm))
        {
            const std::string& fname = _chelper->filepath(fm);
            if ( QMessageBox::No == QMessageBox::question( _chelper->parentWidget(), tr(("Unsaved changes on \"" + fname + "\"").c_str()),
                                                tr("Model has unsaved changes! Close without saving anyway?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
            {
                _cset.erase(fm);
            }   // end if
        }   // end if
    }   // end for
    return !_cset.empty();
}   // end doBeforeAction


bool ActionCloseFaceModels::doAction( FaceControlSet&)
{
    _chelper->close( _cset);
    _cset.clear();
    return true;
}   // end doAction
