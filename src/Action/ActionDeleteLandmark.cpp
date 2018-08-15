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

#include <ActionDeleteLandmark.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ChangeEvents.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionDeleteLandmark;
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionDeleteLandmark::ActionDeleteLandmark( const QString& dn, const QIcon& ico, ActionEditLandmarks* e)
    : FaceAction( dn, ico), _editor(e)
{
}   // end ctor


bool ActionDeleteLandmark::testEnabled( const QPoint*) const
{
    bool enabled = false;
    assert(_editor);
    if ( _editor->isChecked())
    {
        LandmarksInteractor* interactor = _editor->interactor();
        const FaceControl* fc = interactor->hoverModel();
        enabled = isReady( fc) && interactor->hoverID() >= 0;
    }   // end if
    return enabled;
}   // end testEnabled


bool ActionDeleteLandmark::doAction( FaceControlSet& fcs, const QPoint&)
{
    assert(_editor);
    LandmarksInteractor* interactor = _editor->interactor();
    FaceControl* hc = fcs.first();
    assert(hc);
    fcs.clear();
    if ( interactor->deleteLandmark())
        fcs.insert( hc->data());
    return !fcs.empty();
}   // end doAction
