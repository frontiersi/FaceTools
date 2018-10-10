/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;


ActionOrientCameraToFace::ActionOrientCameraToFace( const QString& dn, const QIcon& ico, float d, float r)
    : FaceAction( dn, ico), _distance(d), _urads(r)
{
}   // end ctor


bool ActionOrientCameraToFace::testReady( const FV* fv)
{
    const FaceModel* fm = fv->data();
    fm->lockForRead();
    const bool glmk = fm->centreSet();
    fm->unlock();
    return glmk;
}   // end testReady


bool ActionOrientCameraToFace::doAction( FVS& fset, const QPoint&)
{
    assert(fset.size() == 1);
    const FV* fv = fset.first();
    const FaceModel* fm = fv->data();

    fm->lockForRead();
    RFeatures::Orientation on = fm->orientation();
    cv::Vec3f focus = fm->centre();
    fm->unlock();

    // Rotate the orientation about its up vector by the set amount.
    RFeatures::Transformer transformer( _urads, on.uvec());
    on.rotate( transformer.matrix());

    // Set the camera as needed.
    fv->viewer()->setCamera( focus, on.nvec(), on.uvec(), _distance);

    return true;
}   // end doAction
