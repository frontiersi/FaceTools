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

#include <ActionReflect.h>
#include <ActionInvertNormals.h>
#include <FaceModel.h>
using FaceTools::Action::ActionReflect;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionReflect::ActionReflect( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
}   // end ctor


QString ActionReflect::toolTip() const
{
    return "Reflect through the YZ plane, or if landmarks are present, through the midsagittal (median) plane.";
}   // end toolTip


bool ActionReflect::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


bool ActionReflect::doBeforeAction( Event)
{
    _ev = Event::AFFINE_CHANGE;
    FM* fm = MS::selectedModel();
    fm->lockForRead();
    if ( !fm->currentAssessment()->landmarks().empty())  // Will also have to invert normals if landmarks present
        _ev.add( {Event::LANDMARKS_CHANGE, Event::GEOMETRY_CHANGE});
    fm->unlock();
    return true;
}   // end doBeforeAction


void ActionReflect::doAction( Event)
{
    storeUndo(this, _ev);

    FM* fm = MS::selectedModel();
    fm->lockForWrite();

    cv::Matx44d rmat = cv::Matx44d( -1, 0, 0, 0,
                                     0, 1, 0, 0,
                                     0, 0, 1, 0,
                                     0, 0, 0, 1);

    const Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    // If the model has landmarks, then reflect through the model's median plane.
    if ( !lmks.empty())
    {
        // Translate to origin, reflect through YZ plane, then translate back.
        const cv::Matx44d m = lmks.orientation().asMatrix( lmks.fullMean());
        rmat = m * rmat * m.inv();
        ActionInvertNormals::invertNormals( fm->wmodel());
    }   // end if

    fm->addTransformMatrix(rmat);
    fm->swapLandmarkLaterals();
    fm->unlock();
}   // end doAction


void ActionReflect::doAfterAction( Event)
{
    QString smsg = "Reflected model through YZ plane.";
    if ( !MS::selectedModel()->currentAssessment()->landmarks().empty())
        smsg = "Reflected face through midsagittal plane.";
    MS::showStatus( smsg, 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent(_ev);
}   // end doAfterAction
