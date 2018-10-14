/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <FaceModelManager.h>
#include <QMessageBox>
#include <boost/filesystem.hpp>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseFaceModels;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::FileIO::FMM;


ActionCloseFaceModels::ActionCloseFaceModels( const QString& dname, const QIcon& ico, QWidget* pw)
    : FaceAction( dname, ico), _parent(pw)
{
}   // end ctor


bool ActionCloseFaceModels::doBeforeAction( FVS& fvs, const QPoint&)
{
    FaceModelSet fms = fvs.models();
    fvs.clear();
    for ( FM* fm : fms)
    {
        fm->lockForRead();
        bool inPreferredFormat = FMM::hasPreferredFileFormat(fm);

        // If FaceModel hasn't been saved and the user doesn't want to close it (after prompting), remove from action set.
        bool doclose = false;
        if ( fm->isSaved())
            doclose = true;
        else
        {
            const std::string fname = boost::filesystem::path( FMM::filepath(fm)).filename().string();
            QString msg = tr( ("Model \"" + fname + "\" has unsaved changes! Close anyway?").c_str());
            if ( !inPreferredFormat)
                msg = tr("Model not saved in preferred file format (.3df); close anyway?");

            doclose = QMessageBox::Yes == QMessageBox::question( _parent, tr("Unsaved changes!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        }   // end if
        fm->unlock();

        if ( doclose)
            fvs.insert(fm);
    }   // end for
    return !fvs.empty();
}   // end doBeforeAction
