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

#include <RadialSelectInteractor.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControl;


// public
RadialSelectInteractor::RadialSelectInteractor( FaceModelViewer* viewer)
    : ModelViewerInteractor( viewer), _viewer(viewer), _onDownCamLocked(false)
{
}   // end ctor


void RadialSelectInteractor::rightButtonDown( const QPoint&)
{
}   // end rightButtonDown


void RadialSelectInteractor::leftDoubleClick( const QPoint&)
{
}   // end leftDoubleClick


void RadialSelectInteractor::leftButtonDown( const QPoint& p)
{
    _onDownCamLocked = isCameraLocked();
    setCameraLocked(true);
    setCentre(p);
}   // end leftButtonDown


void RadialSelectInteractor::leftButtonUp( const QPoint&)
{
    setCameraLocked(_onDownCamLocked);
}   // end leftButtonUp


void RadialSelectInteractor::leftDrag( const QPoint&)
{
}   // end leftDrag


// private
void RadialSelectInteractor::setCentre( const QPoint& p)
{
    const FaceControlSet& fcs = _viewer->selected();    // Only look at the selected models when setting the centre
    FaceControl* fc = fcs.find( _viewer->getPointedAt( p));    // fc is a selected model
}   // end setCentre

