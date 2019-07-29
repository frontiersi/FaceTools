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

#include <ActionBackfaceCulling.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Landmark.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionBackfaceCulling;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

namespace {
bool shouldTurnOn( const FM* fm)
{
    const FaceTools::Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    // Don't respond if the landmark isn't present.
    if ( !lmks.hasCode( FaceTools::Landmark::G))
        return false;

    // If the face normal is in the same space half as the orientation normal,
    // then we want to automatically turn on backface culling.
    // Find a polygon at the glabella
    const cv::Vec3f v = lmks.pos( FaceTools::Landmark::G);

    // Calculate the norm given by the ordering of the vertices on the polygon.
    const RFeatures::ObjModel& model = fm->model();
    const int vidx = fm->findVertex(v);
    const cv::Vec3f fnrm = model.calcFaceNorm( *model.faces(vidx).begin());

    // If normal in same direction (positive inner product) as orientation, respond (return true).
    const cv::Vec3f onrm = lmks.orientation().nvec();

    return onrm.dot(fnrm) > 0;
}   // end shouldTurnOn
}   // end namespace


ActionBackfaceCulling::ActionBackfaceCulling( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setCheckable( true, false);
    addTriggerEvent( Event::FACE_DETECTED);
    addTriggerEvent( Event::LOADED_MODEL);
    addTriggerEvent( Event::LANDMARKS_CHANGE);
}   // end ctor


bool ActionBackfaceCulling::checkState( Event e)
{
    bool rval = false;
    const FV* fv = MS::selectedView();
    if ( fv)
    {
        const FM* fm = fv->data();
        if ( fv->backfaceCulling())
        {
            rval = true;
            // Turn off if there was a landmarks change resulting in no landmarks being present (detection reset).
            if ( EventGroup(e).has(Event::LANDMARKS_CHANGE) && fm->currentAssessment()->landmarks().empty())
                rval = false;
        }   // end if
        else
            rval = isTriggerEvent(e) && shouldTurnOn( fm);
    }   // end if

    return rval;
}   // end checkState


bool ActionBackfaceCulling::checkEnable( Event) { return MS::isViewSelected();}   // end checkEnable


void ActionBackfaceCulling::doAction( Event)
{
    const bool ischecked = isChecked();
    const FMV* fmv = MS::selectedView()->viewer();

    // Apply to all FaceViews of all models in the selected FaceView's viewer.
    for ( FV* afv : fmv->attached())
    {
        const FM* fm = afv->data();
        for ( FV* fv : fm->fvs()) // All views of this model
            fv->setBackfaceCulling( ischecked);
    }   // end for
}   // end doAction
