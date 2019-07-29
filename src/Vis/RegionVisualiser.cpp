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

#include <RegionVisualiser.h>
#include <LandmarksManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>           // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <vtkFollower.h>
using FaceTools::Vis::RegionVisualiser;
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::FaceLateral;
using FaceTools::Landmark::LmkList;


RegionVisualiser::RegionVisualiser( int id, const LmkList* lmks0, const LmkList* lmks1)
    : MetricVisualiser( id), _lmks0(lmks0), _lmks1(lmks1) {}


namespace {
void highlight( vtkActor* actor, bool v)
{
    vtkProperty* prop = actor->GetProperty();
    if ( v)
    {
        prop->SetLineWidth(8.0);
        prop->SetColor( 0.8, 0.2, 0.2);
        prop->SetOpacity(0.99);
    }   // end if
    else
    {
        prop->SetLineWidth(2.0);
        prop->SetColor( 0.2, 0.2, 1.0);
        prop->SetOpacity(0.3);
    }   // end else
}   // end highlight


void setActorProperties( vtkActor* actor)
{
    vtkProperty* prop = actor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetAmbient( 1);
    prop->SetDiffuse( 0);
    prop->SetSpecular(0);
    highlight( actor, false);
    actor->SetPickable(false);
}   // end setActorProperties

/*
void setLineColour( vtkActor* actor, const QColor& bg)
{
    QColor col = FaceTools::chooseContrasting( bg);
    vtkProperty* prop = actor->GetProperty();
    prop->SetColor( col.redF(), col.greenF(), col.blueF());
}   // end setLineColour
*/
}   // end namespace


bool RegionVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    return ( _rep0.count(fv) > 0 && _rep0.at(fv) == p) || ( _rep1.count(fv) > 0 && _rep1.at(fv) == p);
}   // end belongs


void RegionVisualiser::syncActorsToData(const FV *fv, const cv::Matx44d &d)
{
    const cv::Matx44d& bmat = fv->data()->model().transformMatrix();
    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK( d * bmat);
    if ( _rep0.count(fv) > 0)
        _rep0.at(fv)->PokeMatrix(vm);
    if ( _rep1.count(fv) > 0)
        _rep1.at(fv)->PokeMatrix(vm);
}   // end syncActorsToData


void RegionVisualiser::setHighlighted( const FV* fv, bool v)
{
    if ( _rep0.count(fv) > 0)
        highlight( _rep0.at(fv), v);
    if ( _rep1.count(fv) > 0)
        highlight( _rep1.at(fv), v);
}   // end setHighlighted


void RegionVisualiser::doSetVisible( const FV* fv, bool v)
{
    if ( _rep0.count(fv) > 0)
        _rep0.at(fv)->SetVisibility(v);
    if ( _rep1.count(fv) > 0)
        _rep1.at(fv)->SetVisibility(v);
}   // end doSetVisible


bool RegionVisualiser::isVisible( const FV *fv) const
{
    bool vis = false;
    if ( _rep0.count(fv) > 0)
        vis = _rep0.at(fv)->GetVisibility() != 0;
    if ( _rep1.count(fv) > 0)
        vis &= _rep1.at(fv)->GetVisibility() != 0;
    return vis;
}   // end isVisible


void RegionVisualiser::checkState( const FV*)
{
    /*
    const QColor bg = fv->viewer()->backgroundColour();
    if ( _rep0.count(fv) > 0)
        setLineColour( _rep0.at(fv), bg);
    if ( _rep1.count(fv) > 0)
        setLineColour( _rep0.at(fv), bg);
    */
}   // end checkState


bool RegionVisualiser::isAvailable( const FM* fm) const
{
    using SLMK = Landmark::SpecificLandmark;
    bool b0 = true;
    if ( _lmks0)
        b0 = std::all_of( std::begin(*_lmks0), std::end(*_lmks0), [fm]( const SLMK& lm){ return fm->currentAssessment()->landmarks().has(lm);});
    bool b1 = true;
    if ( _lmks1)
        b1 = std::all_of( std::begin(*_lmks1), std::end(*_lmks1), [fm]( const SLMK& lm){ return fm->currentAssessment()->landmarks().has(lm);});
    return b0 && b1;
}   // end isAvailable


void RegionVisualiser::doApply( const FV *fv)
{
    if ( _lmks0 && !_lmks0->empty())
        applyActor( fv, _lmks0, _rep0);
    if ( _lmks1 && !_lmks1->empty())
        applyActor( fv, _lmks1, _rep1);
}   // end doApply


void RegionVisualiser::doPurge( const FV *fv)
{
    purgeActor( fv, _rep0);
    purgeActor( fv, _rep1);
}   // end doPurge


// private
void RegionVisualiser::applyActor( const FV *fv, const LmkList* lmks, std::unordered_map<const FV*, vtkActor*>& actors)
{
    const FM* fm = fv->data();
    std::vector<cv::Vec3f> vtxs;
    for ( const auto& lmk : *lmks)
        vtxs.push_back(fm->currentAssessment()->landmarks().pos(lmk));
    vtkActor* actor = actors[fv] = RVTK::VtkActorCreator::generateLineActor(vtxs, true);
    setActorProperties(actor);
    fv->viewer()->add( actor);
}   // end applyActor


// private
void RegionVisualiser::purgeActor( const FV *fv, std::unordered_map<const FV*, vtkActor*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end purgeActor
