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

#include <Action/ActionAlignLandmarks.h>
#include <Interactor/LandmarksHandler.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceModel.h>
#include <FaceTools.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionAlignLandmarks;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using LMAN = FaceTools::Landmark::LandmarksManager;


ActionAlignLandmarks::ActionAlignLandmarks( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    addRefreshEvent( Event::MASK_CHANGE | Event::LANDMARKS_CHANGE | Event::AFFINE_CHANGE);
}   // end ctor


QString ActionAlignLandmarks::toolTip() const
{
    QStringList txt;
    txt << "Centre medial landmarks and set right lateral landmarks to be a";
    txt << "reflection of the left lateral landmarks (through the medial plane).";
    return txt.join("\n");
}   // end toolTip


QString ActionAlignLandmarks::whatsThis() const
{
    QStringList htxt;
    htxt << "Remap landmarks so that the medial landmarks are coincident with the YZ plane";
    htxt << "and the landmarks in the left lateral are reflected through this plane to the";
    htxt << "right lateral. Use this function to help manually place landmarks on new custom";
    htxt << "symmetric coregistration masks. Note that landmarks should already be in their";
    htxt << "approximately required positions.";
    return htxt.join(" ");
}   // end whatsThis


bool ActionAlignLandmarks::isAllowed( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && fm->hasLandmarks() && fm->isAligned();
}   // end isAllowed


bool ActionAlignLandmarks::doBeforeAction( Event)
{
    // TODO Warn about only being applicable for symmetric faces
    storeUndo( this, Event::LANDMARKS_CHANGE | Event::METRICS_CHANGE);
    return true;
}   // end doBeforeAction


namespace {

using namespace FaceTools;

void updateLandmark( FM &fm, int lmid, FaceSide fs, const Vec3f &pos)
{
    fm.setLandmarkPosition( lmid, fs, pos);
    Vis::LandmarksVisualisation &vis = MS::handler<Interactor::LandmarksHandler>()->visualisation();
    for ( const Vis::FV *fv : fm.fvs())
        vis.refreshLandmarkPosition( fv, lmid, fs);
}   // end updateLandmark


void centreMedialLandmarks( FM &fm)
{
    // Set all the medial landmark positions to be at X=0 and iteratively reposition
    // them until change in the x position is <= MAX_XPOS_DIFF.
    const r3d::KDTree &kdt = fm.kdtree();
    const Landmark::LandmarkSet &lmks = fm.currentLandmarks();
    static const float MAX_XPOS_DIFF = 1.0e-8f;
    static const size_t MAX_ITERATIONS = 10;
    const std::unordered_map<int, r3d::Vec3f>& mlmks = lmks.lateral( MID);
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
        updateLandmark( fm, p.first, MID, pos);
    }   // end for
}   // end centreMedialLandmarks


void reflectLateralLandmarks( FM &fm)
{
    const std::unordered_map<int, r3d::Vec3f>& lmks = fm.currentLandmarks().lateral( LEFT);
    for ( const auto &p : lmks)
    {
        r3d::Vec3f pos = p.second;
        pos[0] = -pos[0];
        updateLandmark( fm, p.first, RIGHT, pos);
    }   // end for
}   // end reflectLateralLandmarks

}   // end namespace


void ActionAlignLandmarks::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    centreMedialLandmarks( *fm);
    reflectLateralLandmarks( *fm);
}   // end doAction


Event ActionAlignLandmarks::doAfterAction( Event)
{
    MS::showStatus("Made landmarks laterally symmetric.", 5000);
    return Event::LANDMARKS_CHANGE;
}   // end doAfterAction

