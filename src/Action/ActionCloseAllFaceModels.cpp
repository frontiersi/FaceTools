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
using FaceTools::Action::ActionCloseAllFaceModels;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::CloseFaceModelsHelper;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;


ActionCloseAllFaceModels::ActionCloseAllFaceModels( CloseFaceModelsHelper* chelper)
    : FaceAction(true/*action disabled on other actions executing*/), _chelper(chelper)
{
    setAsync(true);
}   // end ctor


bool ActionCloseAllFaceModels::doBeforeAction( FaceControlSet&)
{
    bool doclose = true; // If any of the open models aren't saved, ask user to confirm.
    const std::unordered_set<FaceModel*>& models = _chelper->opened();
    for ( FaceModel* fm : models)
    {
        if ( !_chelper->isSaved(fm))
        {
            if ( QMessageBox::No == QMessageBox::question( _chelper->parentWidget(), tr("Unsaved changes!"),
                                                tr("Open models(s) have unsaved changes! Close all without saving anyway?"),
                                                QMessageBox::Yes | QMessageBox::No, QMessageBox::No))
            {
                doclose = false;
            }   // end if
            break;  // Question being asked for all models so break
        }   // end if
    }   // end for
    return doclose;
}   // end doBeforeAction


bool ActionCloseAllFaceModels::doAction( FaceControlSet&)
{
    std::unordered_set<FaceModel*> opened = _chelper->opened(); // Copy out
    _chelper->close( opened);
    return true;
}   // end doAction
