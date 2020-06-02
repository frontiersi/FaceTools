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
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/Transformer.h>
#include <r3d/CameraParams.h>
using FaceTools::Action::ActionOrientCameraToFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using MS = FaceTools::Action::ModelSelector;


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico, const QKeySequence& ks, float dp, int ax, float r)
    : FaceAction( dn, ico, ks), _dprop(dp), _raxis(std::max(0, std::min( ax, 2))), _urads(r) {}


bool ActionOrientCameraToFace::isAllowed( Event)
{
    return MS::isViewSelected();
}   // end isAllowed


void ActionOrientCameraToFace::doAction( Event)
{
    const FV *fv = MS::selectedView();
    if ( !fv)
    {
        for ( FMV* vwr : MS::viewers())
        {
            vwr->resetDefaultCamera();
            vwr->setCameraFocus( Vec3f::Zero());
        }   // end for
        _e = Event::CAMERA_CHANGE | Event::ALL_VIEWERS;
    }   // end if
    else
    {
        orientToFace( fv, _dprop, _raxis, _urads);
        _e = Event::CAMERA_CHANGE;
    }   // end else
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
    const Mat4f& T = fm->transformMatrix();
    fm->unlock();

    const r3d::Mat3f R = r3d::Transformer( rads, T.block<3,1>(0,raxis)).matrix().block<3,3>(0,0);
    const r3d::Mat3f nR = R * T.block<3,3>(0,0);
    Vec3f uvec = nR.block<3,1>(0,1);
    uvec.normalize();
    Vec3f nvec = nR.block<3,1>(0,2);
    nvec.normalize();

    const Vec3f focus = T.block<3,1>(0,3); // The point of focus is the model alignment centre

    const r3d::CameraParams cp = fv->viewer()->camera();
    float dst = cp.distance();
    if ( dprop > 0)
        dst = 1.7f * dprop * fm->bounds()[0]->diagonal();
    dst = std::max(200.0f, dst);    // Minimum of 200 in case of unforeseen issue

    const r3d::CameraParams ncp( focus + dst*nvec, focus, uvec, cp.fov());
    fv->viewer()->setCamera( ncp);
}   // end orientToFace
