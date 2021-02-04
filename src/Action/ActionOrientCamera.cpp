/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionOrientCamera.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/Transformer.h>
#include <r3d/CameraParams.h>
using FaceTools::Action::ActionOrientCamera;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::ModelViewer;
using MS = FaceTools::ModelSelect;


ActionOrientCamera::ActionOrientCamera( const QString& dn, const QIcon& ico, const QKeySequence& ks, float dp, int ax, float r)
    : FaceAction( dn, ico, ks), _dprop(dp), _raxis(std::max(0, std::min( ax, 2))), _urads(r) {}


bool ActionOrientCamera::isAllowed( Event) { return MS::isViewSelected();}


namespace {

struct LocalRotation
{
    LocalRotation( const FM &fm, float dprop, int raxis, float rads)
        : _fm(fm), _dprop(dprop)
    {
        const r3d::Mat4f T = fm.transformMatrix();
        focus = T.block<3,1>(0,3); // Focus is model centre
        const r3d::Mat3f R = r3d::Transformer( rads, T.block<3,1>(0,raxis)).matrix().block<3,3>(0,0);
        const r3d::Mat3f nR = R * T.block<3,3>(0,0);
        uvec = nR.block<3,1>(0,1);
        uvec.normalize();
        nvec = nR.block<3,1>(0,2);
        nvec.normalize();
    }   // end ctor

    r3d::Vec3f focus;
    r3d::Vec3f uvec;
    r3d::Vec3f nvec;

    r3d::CameraParams cameraParams( float fov, float d=0.0f) const
    {
        if ( d <= 0.0f)
        {
            const auto &bnds = _fm.bounds()[0];
            const float dim = std::max( bnds->height(), bnds->width());
            d = 8*dim * _dprop * tan(fov * EIGEN_PI/360.0f);
        }   // end if
        return r3d::CameraParams( focus + d*nvec, focus, uvec, fov);
    }   // end cameraParams

private:
    const FM &_fm;
    const float _dprop;
};  // end struct


bool isSameViewVector( const r3d::CameraParams &c0, const r3d::CameraParams &c1)
{
    r3d::Vec3f v0 = c0.pos() - c0.focus();
    v0.normalize();
    r3d::Vec3f v1 = c1.pos() - c1.focus();
    v1.normalize();
    return fabsf( v0.dot(v1) - 1.0f) < 0.0001f;
}   // end isSameViewVector
}   // end namespace


r3d::CameraParams ActionOrientCamera::makeFrontCamera( const FM &fm, float fov, float dscale)
{
    return LocalRotation( fm, dscale, 1, 0.0f).cameraParams( fov);
}   // end makeFrontCamera


bool ActionOrientCamera::doBeforeAction( Event e) { return isAllowed( e);}


void ActionOrientCamera::doAction( Event)
{
    FV *fv = MS::selectedView();
    assert(fv);
    FM::RPtr fm = fv->rdata();

    const r3d::CameraParams ccp = fv->viewer()->camera();  // The current camera
    LocalRotation lrot( *fm, _dprop, _raxis, _urads);
    r3d::CameraParams ncp = lrot.cameraParams( ccp.fov());

    if ( isSameViewVector( ccp, ncp))
    {
        if ( (ccp.focus() - ncp.focus()).norm() > 0.0001f && fabsf(ccp.distance() - ncp.distance()) > 0.0001f)
        {
            // If different focus and distance then use new distance but old focus
            lrot.focus = ccp.focus();
            ncp = lrot.cameraParams( ccp.fov());
        }   // end if
    }   // end if
    else
    {
        // Recalc the camera orientation using the old focus and the
        // existing distance if the view vectors are different.
        lrot.focus = ccp.focus();
        ncp = lrot.cameraParams( ccp.fov(), ccp.distance());
    }   // end else

    fv->viewer()->setCamera( ncp);
    _e = Event::CAMERA_CHANGE;
}   // end doAction


Event ActionOrientCamera::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return _e;
}   // end doAfterAction


void ActionOrientCamera::orient( FV *fv, float dprop, int raxis, float rads)
{
    assert(fv);
    FM::RPtr fm = fv->rdata();
    const float fov = fv->viewer()->camera().fov();
    fv->viewer()->setCamera( LocalRotation( *fm, dprop, raxis, rads).cameraParams( fov));
}   // end orient
