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

#include <EuclideanDistanceVisualiser.h>
#include <LandmarksManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>           // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
using FaceTools::Vis::EuclideanDistanceVisualiser;
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::FaceLateral;
using FaceTools::Landmark::LmkList;


EuclideanDistanceVisualiser::EuclideanDistanceVisualiser( int id, const LmkList* l0, const LmkList* l1)
    : MetricVisualiser( id), _lmks0(l0), _lmks1(l1)
{
}   // end ctor


namespace {
void highlight( vtkActor* actor, bool v)
{
    assert(actor);
    vtkProperty* prop = actor->GetProperty();
    if ( v)
    {
        prop->SetLineWidth(8.0);
        prop->SetColor( 0.8, 0.0, 0.2);
        prop->SetOpacity(0.99);
    }   // end if
    else
    {
        prop->SetLineWidth(2.0);
        prop->SetColor( 0.2, 0.2, 1.0);
        prop->SetOpacity(0.3);
    }   // end else
}   // end highlight

/*
void setLineColour( vtkActor* actor, const QColor& bg)
{
    QColor col = FaceTools::chooseContrasting( bg);
    vtkProperty* prop = actor->GetProperty();
    prop->SetColor( col.redF(), col.greenF(), col.blueF());
}   // end setLineColour
*/
}   // end namespace


bool EuclideanDistanceVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _actor0.count(fv) > 0 && _actor0.at(fv) == p)
        return true;
    else if ( _actor1.count(fv) > 0 && _actor1.at(fv) == p)
        return true;
    return false;
}   // end belongs


void EuclideanDistanceVisualiser::syncActorsToData(const FV *fv, const cv::Matx44d &d)
{
    const cv::Matx44d& bmat = fv->data()->model().transformMatrix();
    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK( d * bmat);
    if ( _actor0.count(fv) > 0)
        _actor0.at(fv)->PokeMatrix( vm);
    if ( _actor1.count(fv) > 0)
        _actor1.at(fv)->PokeMatrix( vm);
}   // end syncActorsToData


void EuclideanDistanceVisualiser::setHighlighted( const FV* fv, bool v)
{
    if ( _actor0.count(fv) > 0)
        highlight( _actor0.at(fv), v);
    if ( _actor1.count(fv) > 0)
        highlight( _actor1.at(fv), v);
}   // end setHighlighted


void EuclideanDistanceVisualiser::doSetVisible( const FV* fv, bool v)
{
    if ( _actor0.count(fv) > 0)
        _actor0.at(fv)->SetVisibility(v);
    if ( _actor1.count(fv) > 0)
        _actor1.at(fv)->SetVisibility(v);
}   // end doSetVisible


bool EuclideanDistanceVisualiser::isVisible(const FV *fv) const
{
    bool isvis = false;
    if ( _actor0.count(fv) > 0)
        isvis = _actor0.at(fv)->GetVisibility() != 0;
    return isvis;
}   // end isVisible


void EuclideanDistanceVisualiser::checkState( const FV* fv)
{
    /*
    const QColor bg = fv->viewer()->backgroundColour();
    if ( _actor0.count(fv) > 0)
        setLineColour( _actor0.at(fv), bg);
    if ( _actor1.count(fv) > 0)
        setLineColour( _actor1.at(fv), bg);
    */
}   // end checkState


bool EuclideanDistanceVisualiser::isAvailable( const FM* fm) const
{
    using SLMK = Landmark::SpecificLandmark;
    bool b0 = true;
    if ( _lmks0)
        b0 = std::all_of( std::begin(*_lmks0), std::end(*_lmks0), [fm]( const SLMK& lm){ return fm->landmarks().has(lm);});
    bool b1 = true;
    if ( _lmks1)
        b1 = std::all_of( std::begin(*_lmks1), std::end(*_lmks1), [fm]( const SLMK& lm){ return fm->landmarks().has(lm);});
    return b0 && b1;
}   // end isAvailable


void EuclideanDistanceVisualiser::doApply( const FV *fv)
{
    if ( _lmks0 && _lmks0->size() == 2)
        applyActor( fv, _lmks0, _actor0);
    if ( _lmks1 && _lmks1->size() == 2)
        applyActor( fv, _lmks1, _actor1);
}   // end doApply


void EuclideanDistanceVisualiser::doPurge( const FV *fv)
{
    purgeActor( fv, _actor0);
    purgeActor( fv, _actor1);
}   // end doPurge


// private
void EuclideanDistanceVisualiser::applyActor( const FV *fv, const LmkList* ll, std::unordered_map<const FV*, vtkActor*>& actors)
{
    const FM* fm = fv->data();

    assert( ll);
    assert( !ll->empty());
    assert( fm->landmarks().has(ll->front()));
    assert( fm->landmarks().has(ll->back()));
    const std::vector<cv::Vec3f> vtxs = { fm->landmarks().pos(ll->front()), fm->landmarks().pos(ll->back())};

    vtkActor* actor = actors[fv] = RVTK::VtkActorCreator::generateLineActor( vtxs);
    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);

    property->SetAmbient( 1);
    property->SetDiffuse( 0);
    property->SetSpecular(0);
    highlight( actor, false);
    actor->SetPickable(false);

    fv->viewer()->add( actor);
}   // end applyActor


// private
void EuclideanDistanceVisualiser::purgeActor( const FV *fv, std::unordered_map<const FV*, vtkActor*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end purgeActor
