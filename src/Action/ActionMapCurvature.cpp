/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <ActionMapCurvature.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <LandmarkSet.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::ActionMapCurvature;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionMapCurvature::ActionMapCurvature( QProgressBar* pb)
    : FaceAction(true/*disable before other*/), _icon( ":/icons/ANGLES")
{
    addChangeTo( SURFACE_METRICS_CALCULATED);
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_CHANGED);
    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


ActionMapCurvature::~ActionMapCurvature() { _cmaps.clear();}


RFeatures::ObjModelCurvatureMetrics::Ptr ActionMapCurvature::metrics( FaceControl* fc)
{
    FaceModel* fm = fc->data();
    if ( testReady(fc))
        process(fc);    // Calls toAction on this
    RFeatures::ObjModelCurvatureMetrics::Ptr p;
    if ( _cmaps.count(fm) > 0)
        p = _cmaps.at(fm);
    return p;
}   // end metrics


bool ActionMapCurvature::testReady( FaceControl* fc)
{
    FaceModel* fm = fc->data();
    return fm->landmarks().has(FaceTools::Landmarks::NASAL_TIP) && _cmaps.count(fm) == 0;
}   // end testReady


bool ActionMapCurvature::doAction( FaceControlSet& rset)
{
    const FaceModelSet& fmset = rset.models();
    for ( FaceModel* fm : fmset)
    {
        assert(_cmaps.count(fm) == 0);
        using namespace RFeatures;
        const LandmarkSet& lmks = fm->landmarks();
        int vidx = fm->kdtree()->find( lmks.pos(FaceTools::Landmarks::NASAL_TIP));
        ObjModel::Ptr m = fm->model();
        int sfid = *m->getFaceIds(vidx).begin();
        ObjModelCurvatureMap::Ptr cm = ObjModelCurvatureMap::create(m, sfid);   // Create the curvature mapping
        _cmaps[fm] = ObjModelCurvatureMetrics::create(cm);  // Create the metrics from the raw curvature data
    }   // end for
    return true;
}   // end doAction


void ActionMapCurvature::respondToChange( FaceControl* fc)
{
    FaceAction* sending = qobject_cast<FaceAction*>(sender());
    if ( sending->changeEvents().count( MODEL_GEOMETRY_CHANGED) && _cmaps.count(fc->data()) > 0)
    {
        burn(fc);
        process(fc);    // Re-map the curvature
    }   // end if
    FaceAction::respondToChange(fc);
}   // end respondToChange


void ActionMapCurvature::burn( const FaceControl* fc)
{
    if ( _cmaps.count(fc->data()) > 0)
        _cmaps.erase(fc->data());
}   // end burn
