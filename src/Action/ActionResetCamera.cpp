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
#include <algorithm>
using FaceTools::Action::ActionResetCamera;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;


ActionResetCamera::ActionResetCamera( const QString& dn, const QIcon& ico, FaceModelViewer *mv)
    : FaceAction( dn, ico)
{
    if ( mv)
        addViewer(mv);
}   // end ctor


bool ActionResetCamera::doAction( FaceControlSet &fset)
{
    if ( _viewers.empty())
    {
        FaceViewerSet vwrs = fset.viewers();
        for ( FaceModelViewer* v : vwrs)
            v->resetCamera();
    }   // end if
    else
    {
        std::for_each( std::begin(_viewers), std::end(_viewers), [](auto v){ v->resetCamera();});
        std::for_each( std::begin(_viewers), std::end(_viewers), [](auto v){ v->updateRender();});
    }   // end else
    return true;
}   // end doAction
