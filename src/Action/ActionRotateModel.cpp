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

#include <ActionRotateModel.h>
#include <Transformer.h>
#include <FaceModel.h>
using FaceTools::Action::ActionRotateModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionRotateModel::ActionRotateModel( const QString &dn, const QIcon& ico, const cv::Vec3d& raxis, double degs)
    : FaceAction( dn, ico)
{
    _rmat = RFeatures::Transformer( degs * CV_PI/180, raxis).matrix();
}   // end ctor


QString ActionRotateModel::toolTip() const
{
    return "Rotate through the given axis, or with respect to the head if landmarks are present.";
}   // end toolTip


bool ActionRotateModel::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


void ActionRotateModel::doAction( Event)
{
    storeUndo( this, Event::AFFINE_CHANGE);
    FM* fm = MS::selectedModel();
    fm->lockForWrite();

    cv::Matx44d rmat = _rmat;
    // If the model has landmarks, then rotate with respect to the head's orientation
    // otherwise simply rotate about the defined axis.
    const Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    if ( !lmks.empty())
    {
        // Translate to origin, rotate, then translate back.
        const RFeatures::Orientation on = lmks.orientation();
        const cv::Matx44d m = on.asMatrix( lmks.fullMean());
        rmat = m * _rmat * m.inv();
    }   // end else

    fm->addTransformMatrix(rmat);
    fm->unlock();
}   // end doAction


void ActionRotateModel::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( Event::AFFINE_CHANGE);
}   // end doAfterAction

