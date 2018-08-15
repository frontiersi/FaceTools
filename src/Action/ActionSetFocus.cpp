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

#include <ActionSetFocus.h>
#include <ActionSynchroniseCameraMovement.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionSetFocus;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


// public
ActionSetFocus::ActionSetFocus( const QString& dn) : FaceAction( dn)
{
}   // end ctor


bool ActionSetFocus::testEnabled( const QPoint* p) const
{
    bool enabled = false;
    if ( p && ready1())
        enabled = ready1()->view()->isPointOnFace(*p) != nullptr;
    return enabled;
}   // end testEnabled


bool ActionSetFocus::doAction( FaceControlSet&, const QPoint& p)
{
    FaceControl* fc = ready1();
    assert(fc);
    cv::Vec3f nf;
    const bool onModel = fc->view()->pointToFace( p, nf);
    assert(onModel);    // Must be or couldn't have been ready!
    fc->viewer()->setFocus(nf);
    FaceTools::Action::ActionSynchroniseCameraMovement::sync();
    return true;
}   // end doAction
