/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionCentreModel.h>
#include <Action/ActionOrientCameraToFace.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <r3d/Transformer.h>
using FaceTools::Action::ActionCentreModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionCentreModel::ActionCentreModel( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks) {}


bool ActionCentreModel::isAllowed( Event)
{
    if ( !MS::isViewSelected())
        return false;
    const FM* fm = MS::selectedModel();
    const Vec3f centre = fm->bounds()[0]->centre();
    return centre.squaredNorm() > 0.00001f;
}   // end isAllowedd


void ActionCentreModel::doAction( Event)
{
    storeUndo(this, Event::AFFINE_CHANGE | Event::CAMERA_CHANGE);
    FM* fm = MS::selectedModel();
    Mat4f cT = Mat4f::Identity();
    cT.block<3,1>(0,3) = -fm->bounds()[0]->centre();
    fm->addTransformMatrix( cT);
}   // end doAction


Event ActionCentreModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    ActionOrientCameraToFace::orientToFace( MS::selectedView());
    MS::showStatus("Centred model.", 5000);
    return Event::AFFINE_CHANGE | Event::CAMERA_CHANGE;
}   // end doAfterAction

