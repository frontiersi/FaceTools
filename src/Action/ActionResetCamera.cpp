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

#include <ActionResetCamera.h>
#include <FaceModelViewer.h>
#include <ChangeEvents.h>
#include <algorithm>
using FaceTools::Action::ActionResetCamera;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionResetCamera::ActionResetCamera( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico, true)
{
    addChangeTo( VIEW_CHANGE);
}   // end ctor


bool ActionResetCamera::doAction( FaceControlSet& fset)
{
    FaceViewerSet viewers = fset.viewers();
    std::for_each(std::begin(viewers), std::end(viewers), [](auto v){ v->resetCamera();});
    return true;
}   // end doAction
