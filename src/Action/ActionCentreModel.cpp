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

#include <Action/ActionCentreModel.h>
#include <Action/ActionOrientCamera.h>
using FaceTools::Action::ActionCentreModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionCentreModel::ActionCentreModel( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    addRefreshEvent( Event::MESH_CHANGE | Event::AFFINE_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionCentreModel::isAllowed( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && !fm->bounds()[0]->centre().isZero();
}   // end isAllowed


void ActionCentreModel::doAction( Event)
{
    storeUndo(this, Event::AFFINE_CHANGE | Event::CAMERA_CHANGE);
    FM::WPtr fm = MS::selectedModelScopedWrite();
    Mat4f cT = Mat4f::Identity();
    cT.block<3,1>(0,3) = -fm->bounds()[0]->centre();
    fm->addTransformMatrix( cT);
}   // end doAction


Event ActionCentreModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    ActionOrientCamera::orient( MS::selectedView(), 1);
    MS::showStatus("Centred model.", 5000);
    return Event::AFFINE_CHANGE | Event::CAMERA_CHANGE;
}   // end doAfterAction

