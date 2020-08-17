/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionAlignLandmarks.h>
#include "Action/ActionUpdateMeasurements.h"
#include <LndMrk/LandmarksManager.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAlignLandmarks;
using FaceTools::Action::Event;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionAlignLandmarks::ActionAlignLandmarks( const QString& dn, const QIcon& ico, LandmarksHandler::Ptr handler)
    : FaceAction(dn, ico), _handler(handler)
{
}   // end ctor


QString ActionAlignLandmarks::toolTip() const
{
    return "Centre medial landmarks and reflect right lateral landmarks to left lateral.";
}   // end toolTip


QString ActionAlignLandmarks::whatsThis() const
{
    QStringList htxt;
    htxt << "Remap landmarks so that the medial landmarks are coincident with the YZ plane";
    htxt << "and the landmarks in the right lateral are reflected through this plane to the";
    htxt << "left lateral. Use this function to help manually place landmarks on new custom";
    htxt << "symmetric coregistration masks. Note that landmarks should already be in their";
    htxt << "approximately required positions.";
    return htxt.join(" ");
}   // end whatsThis


bool ActionAlignLandmarks::isAllowed( Event)
{
    const FM* fm = MS::selectedModel();
    return fm && fm->hasLandmarks() && fm->isAligned();
}   // end isAllowedd


bool ActionAlignLandmarks::doBeforeAction( Event)
{
    // TODO Warn about only being applicable for symmetric faces
    storeUndo( this, Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE);
    return true;
}   // end doBeforeAction


namespace {

void centreMedialLandmarks( FM *fm)
{
    // Set all the medial landmark positions to be at X=0 and iteratively reposition
    // them until change in the x position is <= MAX_XPOS_DIFF.
    const r3d::KDTree &kdt = fm->kdtree();
    const FaceTools::Landmark::LandmarkSet &lmks = fm->currentLandmarks();
    static const float MAX_XPOS_DIFF = 1.0e-8f;
    static const size_t MAX_ITERATIONS = 10;
    const std::unordered_map<int, r3d::Vec3f>& mlmks = lmks.lateral( FaceTools::MID);
    for ( const auto &p : mlmks)
    {
        r3d::Vec3f pos = p.second;
        size_t i = 0;
        while ( (fabsf(pos[0]) > MAX_XPOS_DIFF) && (i < MAX_ITERATIONS))
        {
            pos[0] = 0.0f;
            pos = FaceTools::toSurface( kdt, pos);
            i++;
        }   // end while
        pos[0] = 0.0f;
        fm->setLandmarkPosition( p.first, FaceTools::MID, pos);
    }   // end for
}   // end centreMedialLandmarks


void reflectLateralLandmarks( FM *fm)
{
    const std::unordered_map<int, r3d::Vec3f>& lmks = fm->currentLandmarks().lateral( FaceTools::LEFT);
    for ( const auto &p : lmks)
    {
        r3d::Vec3f pos = p.second;
        pos[0] = -pos[0];
        fm->setLandmarkPosition( p.first, FaceTools::RIGHT, pos);
    }   // end for
}   // end reflectLateralLandmarks

}   // end namespace


void ActionAlignLandmarks::doAction( Event)
{
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    centreMedialLandmarks( fm);
    reflectLateralLandmarks( fm);
    for ( const Vis::FV *fv : fm->fvs())
        for ( int lmid : LMAN::ids())
            _handler->visualisation().refreshLandmark( fv, lmid);
    ActionUpdateMeasurements::updateAllMeasurements( fm);
    fm->unlock();
}   // end doAction


Event ActionAlignLandmarks::doAfterAction( Event)
{
    MS::showStatus("Made landmarks laterally symmetric.", 5000);
    return Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE;
}   // end doAfterAction

