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
#include <InteractiveModelViewer.h>
#include <FaceView.h>
using FaceTools::ActionSetFocus;
using FaceTools::FaceControl;
using FaceTools::ModelViewer;

ActionSetFocus::ActionSetFocus()
    : FaceTools::FaceAction(), _fcont(NULL)
{
    init();
    setEnabled(false);
}   // end ctor


void ActionSetFocus::addController( FaceControl* fcont)
{
    connect( fcont, &FaceControl::onExitingModel, this, &ActionSetFocus::checkEnable);
    connect( fcont, &FaceControl::onEnteringModel, this, &ActionSetFocus::checkEnable);
}   // end addController


void ActionSetFocus::removeController( FaceControl* fcont)
{
    _fcont = NULL;
    setEnabled(false);
    fcont->disconnect(this);
}   // end removeController


bool ActionSetFocus::doAction()
{
    assert( _fcont);
    assert( _fcont->getViewer());
    cv::Vec3f nf;
    const QPoint& p = _fcont->getViewer()->getMouseCoords();
    const bool onModel = _fcont->getViewer()->calcSurfacePosition( _fcont->getView()->getActor(), p, nf);
    assert(onModel);
    _fcont->getViewer()->setFocus(nf);
    _fcont->getViewer()->updateRender();
    return true;
}   // end doAction


void ActionSetFocus::checkEnable()
{
    _fcont = qobject_cast<FaceControl*>( sender());
    assert(_fcont);
    if ( !_fcont->isPointedAt())
        _fcont = NULL;
    setEnabled( _fcont);
}   // end checkEnable

