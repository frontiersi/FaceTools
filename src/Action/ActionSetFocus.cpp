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
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionSetFocus;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
typedef FaceTools::Interactor::FaceEntryExitInteractor FEEI;


// public
ActionSetFocus::ActionSetFocus( const QString& dn) : FaceAction( dn, true), _fcont(NULL)
{
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( MODEL_TRANSFORMED);
    addRespondTo( CAMERA_POSITION_CHANGED);
    addRespondTo( CAMERA_ORIENTATION_CHANGED);
    addChangeTo( CAMERA_FOCUS_CHANGED);
    connect( &_interactor, &FEEI::onEnterModel, [this](auto fc){ _fcont = fc; this->respondToChange();});
    connect( &_interactor, &FEEI::onLeaveModel, [this](){ _fcont = NULL; this->respondToChange();});
}   // end ctor


bool ActionSetFocus::testEnabled() { return _fcont != NULL;}


bool ActionSetFocus::doAction( FaceControlSet&)
{
    assert(_fcont);
    FaceTools::ModelViewer* mv = _fcont->viewer();
    assert( mv);
    cv::Vec3f nf;
    const QPoint p = mv->getMouseCoords();
    bool onModel = mv->calcSurfacePosition( _fcont->view()->surfaceActor(), p, nf);
    if ( !onModel)
        onModel = mv->calcSurfacePosition( _fcont->view()->textureActor(), p, nf);
    assert(onModel);    // Must be or couldn't have been ready!
    mv->setFocus(nf);
    return true;
}   // end doAction
