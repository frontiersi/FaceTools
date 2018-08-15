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

#include <ActionCloseAllFaceModels.h>
#include <QMessageBox>
#include <algorithm>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceModel;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( const QString& dname, FaceModelManager* fmm, QWidget* pw)
    : FaceAction( dname), _fmm(fmm), _parent(pw)
{
}   // end ctor


bool ActionCloseAllFaceModels::doBeforeAction( FaceControlSet& fcs, const QPoint&)
{
    bool doclose = true; // If any of the open models aren't saved, ask user to confirm.
    const FaceModelSet& models = _fmm->opened();
    for ( FaceModel* fm : models)
    {
        if ( !fm->isSaved() || (!_fmm->hasPreferredFileFormat(fm) && fm->hasMetaData()))
        {
            static const QString msg = tr("Model(s) have unsaved changes! Close all anyway?");
            if ( QMessageBox::No == QMessageBox::question( _parent, tr("Unsaved changes!"), msg,
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
            {
                doclose = false;
            }   // end if
            break;  // Question being asked on behalf of all models so break
        }   // end if
    }   // end for

    // Populate fcs with FaceControl instances from all open models.
    if ( doclose)
    {
        for ( FaceModel* fm : models)
        {
            const FaceControlSet& fcs0 = fm->faceControls();
            std::for_each( std::begin(fcs0), std::end(fcs0), [&](auto fc){ fcs.insert(fc);});
        }   // end for
    }   // end if

    return doclose;
}   // end doBeforeAction
