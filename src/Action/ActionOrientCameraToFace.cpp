/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionOrientCameraToFace.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <Transformer.h>    // RFeatures
#include <algorithm>
using FaceTools::Action::ActionOrientCameraToFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using MS = FaceTools::Action::ModelSelector;


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico, double d, double r, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _distance(d), _urads(r)
{
}   // end ctor


bool ActionOrientCameraToFace::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


void ActionOrientCameraToFace::doAction(Event)
{
    orientToFace( MS::selectedView(), _distance, _urads);
    emit onEvent( Event::CAMERA_CHANGE);
}   // end doAction


void ActionOrientCameraToFace::orientToFace( const FV *fv, double dist, double rads)
{
    const FM* fm = fv->data();

    fm->lockForRead();
    const cv::Vec3f focus = fm->centre();
    RFeatures::Orientation on = fm->orientation();
    fm->unlock();

    // Rotate the orientation about its up vector by the set amount.
    on.rotate( RFeatures::Transformer( rads, on.uvec()).matrix());

    // Set the camera as needed.
    fv->viewer()->setCamera( focus, on.nvec(), on.uvec(), dist);
}   // end orientToFace
