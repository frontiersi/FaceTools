/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionOrientCameraToFace.h>
#include <Action/ActionSetParallelProjection.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/Transformer.h>
#include <r3d/CameraParams.h>
using FaceTools::Action::ActionOrientCameraToFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::ModelViewer;
using MS = FaceTools::Action::ModelSelector;


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico, const QKeySequence& ks, float dp, int ax, float r)
    : FaceAction( dn, ico, ks), _dprop(dp), _raxis(std::max(0, std::min( ax, 2))), _urads(r) {}


bool ActionOrientCameraToFace::isAllowed( Event) { return MS::isViewSelected();}


namespace {
r3d::CameraParams calcNewCameraParams( const FM *fm, float dprop, int raxis, float rads)
{
    const r3d::Mat4f& T = fm->transformMatrix();
    const r3d::Mat3f R = r3d::Transformer( rads, T.block<3,1>(0,raxis)).matrix().block<3,3>(0,0);
    const r3d::Mat3f nR = R * T.block<3,3>(0,0);
    r3d::Vec3f uvec = nR.block<3,1>(0,1);
    uvec.normalize();
    r3d::Vec3f nvec = nR.block<3,1>(0,2);
    nvec.normalize();
    const r3d::Vec3f focus = T.block<3,1>(0,3); // The point of focus is the model alignment centre
    const float dst = std::max( 200.0f, 1.7f * dprop * fm->bounds()[0]->diagonal());    // Minimum of 200 in case of unforeseen issue
    return r3d::CameraParams( focus + dst*nvec, focus, uvec);
}   // end calcNewCameraParams
}   // end namespace


void ActionOrientCameraToFace::doAction( Event)
{
    const FV *fv = MS::selectedView();
    assert(fv);
    const FM *fm = fv->data();

    const r3d::CameraParams ccp = fv->viewer()->camera();  // The current camera
    fm->lockForRead();
    r3d::CameraParams ncp = calcNewCameraParams( fv->data(), _dprop, _raxis, _urads);
    fm->unlock();
    ncp.setFoV( ccp.fov());

    // If focus is different between the new and old camera or the direction of the focal vector
    // of the new camera position is not inline with the old one, then we just adjust the orientation
    // without changing the current distance.
    float d = ncp.distance();
    if ( ccp.focus() != ncp.focus())
        d = ccp.distance(); // Focus different so keep old distance
    else
    {
        Vec3f v0 = ccp.pos() - ccp.focus();
        v0.normalize();
        Vec3f v1 = ncp.pos() - ncp.focus();
        v1.normalize();
        if ( v0.dot(v1) != 1.0f)    // View vectors not in line so keep old distance
            d = ccp.distance();
    }   // end else

    // If using the new distance, also reset to perspective projection
    /*
    if ( d == ncp.distance())
        ActionSetParallelProjection::setParallelProjection( false);
    */

    ncp.setPositionFromFocus( d);
    fv->viewer()->setCamera( ncp);
    _e = Event::CAMERA_CHANGE;
}   // end doAction


Event ActionOrientCameraToFace::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return _e;
}   // end doAfterAction


void ActionOrientCameraToFace::orientToFace( const FV *fv, float dprop, int raxis, float rads)
{
    assert(fv);
    const FM* fm = fv->data();
    fm->lockForRead();
    r3d::CameraParams ncp = calcNewCameraParams( fm, dprop, raxis, rads);
    fm->unlock();
    const r3d::CameraParams ccp = fv->viewer()->camera();
    ncp.setFoV( ccp.fov());
    fv->viewer()->setCamera( ncp);
}   // end orientToFace
