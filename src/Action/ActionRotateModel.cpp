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

#include <Action/ActionRotateModel.h>
#include <r3d/Transformer.h>
#include <FaceModel.h>
using FaceTools::Action::ActionRotateModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vec3f;
using MS = FaceTools::Action::ModelSelector;


ActionRotateModel::ActionRotateModel( const QString &dn, const QIcon& ico, const Vec3f& raxis, float degs)
    : FaceAction( dn, ico)
{
    _rmat = r3d::Transformer( degs * EIGEN_PI/180, raxis).matrix();
}   // end ctor


QString ActionRotateModel::toolTip() const
{
    return "Rotate through the given axis, or with respect to the head if landmarks are present.";
}   // end toolTip


bool ActionRotateModel::isAllowed( Event)
{
    return MS::isViewSelected();
}   // end isAllowedd


void ActionRotateModel::doAction( Event)
{
    storeUndo( this, Event::AFFINE_CHANGE);
    FM* fm = MS::selectedModel();
    fm->lockForWrite();

    Mat4f rmat;
    // If the model has alignment defined, rotate with respect to that
    // otherwise simply rotate about the defined axis.
    if ( fm->hasLandmarks())
        rmat = fm->transformMatrix() * _rmat * fm->inverseTransformMatrix();
    else
        rmat = _rmat;

    fm->addTransformMatrix(rmat);
    fm->unlock();
}   // end doAction


Event ActionRotateModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return Event::AFFINE_CHANGE;
}   // end doAfterAction
