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

#include <ActionTransformToStandardPosition.h>
#include <ActionOrientCameraToFace.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceView.h>
using FaceTools::Action::ActionTransformToStandardPosition;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionTransformToStandardPosition::ActionTransformToStandardPosition( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
}   // end ctor


QString ActionTransformToStandardPosition::toolTip() const
{
    return "Transform and orient the selected model according to its landmarks (if present) or to its last saved position.";
}   // end toolTip


QString ActionTransformToStandardPosition::whatsThis() const
{
    QStringList htext;
    htext << "Reposition the selected model back to its last saved position and orientation,";
    htext << "or if the model has facial landmarks, use these to orient the face in an upright";
    htext << "position with the mean of the landmarks coincident with the world coordinates origin.";
    return htext.join(" ");
}   // end whatsThis


bool ActionTransformToStandardPosition::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    if (!fm)
        return false;
    using namespace RFeatures;
    fm->lockForRead();
    cv::Vec3f centre = fm->centre();
    Orientation on = fm->orientation();
    fm->unlock();
    // Enable only if the orientation is
    static const double MINF = 1e-6;
    return (l2sq( centre) > MINF) || (l2sq( on.nvec() - cv::Vec3f(0,0,1)) > MINF) || (l2sq( on.uvec() - cv::Vec3f(0,1,0)) > MINF);
}   // end checkEnabled


void ActionTransformToStandardPosition::doAction( Event)
{
    storeUndo(this, {Event::AFFINE_CHANGE, Event::CAMERA_CHANGE});
    const FV* fv = MS::selectedView();
    FM* fm = fv->data();
    fm->lockForWrite();
    const cv::Matx44d bmat = fm->orientation().asMatrix( fm->centre());
    fm->addTransformMatrix( bmat.inv());
    fm->unlock();
    ActionOrientCameraToFace::orientToFace( fv, double(DEFAULT_CAMERA_DISTANCE));
}   // end doAction


void ActionTransformToStandardPosition::doAfterAction( Event)
{
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( {Event::AFFINE_CHANGE, Event::CAMERA_CHANGE});
    QString msg = "Model orientation and position restored.";
    if ( !MS::selectedModel()->currentAssessment()->landmarks().empty())
        msg = "Transformed to detected face orientation.";
    MS::showStatus( msg, 5000);
}   // end doAfterAction

