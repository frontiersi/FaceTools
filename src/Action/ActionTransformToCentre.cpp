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

#include <ActionTransformToCentre.h>
#include <ActionResetCamera.h>
#include <Transformer.h>  // RFeatures
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
#include <VtkTools.h>
using FaceTools::Action::ActionTransformToCentre;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionTransformToCentre::ActionTransformToCentre( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
}   // end ctor


bool ActionTransformToCentre::checkEnable( Event)
{
    if ( !MS::isViewSelected())
        return false;
    using namespace RFeatures;
    const FM* fm = MS::selectedModel();
    fm->lockForRead();
    cv::Vec3f centre = fm->bounds()[0]->centre();
    fm->unlock();
    static const double MINF = 0.00001;
    return l2sq( centre) > MINF;
}   // end checkEnabled


void ActionTransformToCentre::doAction( Event)
{
    storeUndo(this, {Event::AFFINE_CHANGE, Event::CAMERA_CHANGE});
    const FV* fv = MS::selectedView();
    FM* fm = fv->data();

    fm->lockForWrite();
    fm->addTransformMatrix( RFeatures::Transformer( -fm->bounds()[0]->centre()).matrix());
    fm->unlock();
    ActionResetCamera::resetCamera( fv);
}   // end doAction


void ActionTransformToCentre::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( {Event::AFFINE_CHANGE, Event::CAMERA_CHANGE});
    MS::showStatus("Centred model.", 5000);
}   // end doAfterAction

