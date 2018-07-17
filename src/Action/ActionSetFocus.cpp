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
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


// public
ActionSetFocus::ActionSetFocus( const QString& dn, FEEI* feei) : FaceAction( dn)
{
    connect( feei, &FEEI::onEnterModel, [this](auto fc){ this->resetReady(fc);});
    connect( feei, &FEEI::onLeaveModel, [this](auto fc){ this->clearReady();});
}   // end ctor


bool ActionSetFocus::doAction( FaceControlSet& fset)
{
    assert(fset.size() == 1);
    FaceControl* fc = fset.first();
    FaceTools::ModelViewer* mv = fc->viewer();
    cv::Vec3f nf;
    const QPoint p = mv->getMouseCoords();
    bool onModel = mv->calcSurfacePosition( fc->view()->surfaceActor(), p, nf);
    if ( !onModel)
        onModel = mv->calcSurfacePosition( fc->view()->textureActor(), p, nf);
    assert(onModel);    // Must be or couldn't have been ready!
    mv->setFocus(nf);
    return true;
}   // end doAction


void ActionSetFocus::doAfterAction( ChangeEventSet& cs, const FaceControlSet& fcs, bool v)
{
    cs.insert(VIEW_CHANGE);
}   // end doAfterAction
