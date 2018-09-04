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

#include <ActionCloseAllFaceModels.h>
#include <QMessageBox>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( const QString& dname, FaceModelManager* fmm, QWidget* pw)
    : FaceAction( dname), _fmm(fmm), _parent(pw)
{
}   // end ctor


bool ActionCloseAllFaceModels::doBeforeAction( FVS& fvs, const QPoint&)
{
    bool doclose = true; // If any of the open models aren't saved, ask user to confirm.
    const FMS& models = _fmm->opened();
    for ( FM* fm : models)
    {
        fm->lockForRead();
        if ( !fm->isSaved() || (!_fmm->hasPreferredFileFormat(fm) && fm->hasMetaData()))
        {
            static const QString msg = tr("Model(s) have unsaved changes! Close all anyway?");
            if ( QMessageBox::No == QMessageBox::question( _parent, tr("Unsaved changes!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
                doclose = false;
        }   // end if
        fm->unlock();
        if ( !doclose)
            break;
    }   // end for

    if ( doclose)
    {
        for ( FM* fm : models)
            fvs.insert(fm);
    }   // end if

    return doclose;
}   // end doBeforeAction
