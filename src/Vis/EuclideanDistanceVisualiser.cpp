/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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


bool EuclideanDistanceVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _actor0.count(fv) > 0 && _actor0.at(fv) == p)
        return true;
    else if ( _actor1.count(fv) > 0 && _actor1.at(fv) == p)
        return true;
    return false;
}   // end belongs


void EuclideanDistanceVisualiser::pokeTransform( const FV *fv, const vtkMatrix4x4* vm)
{
    if ( _actor0.count(fv) > 0)
        _actor0.at(fv)->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));
    if ( _actor1.count(fv) > 0)
        _actor1.at(fv)->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));
}   // end pokeTransform


void EuclideanDistanceVisualiser::fixTransform( const FV *fv)
{
    if ( _actor0.count(fv) > 0)
    {
        vtkActor* a = _actor0.at(fv);
        RVTK::transform( a, a->GetMatrix());
    }   // end if
    if ( _actor1.count(fv) > 0)
    {
        vtkActor* a = _actor1.at(fv);
        RVTK::transform( a, a->GetMatrix());
    }   // end if
}   // end fixTransform


namespace {

void highlight( vtkActor* actor, bool v)
{
    assert(actor);
    vtkProperty* prop = actor->GetProperty();
    if ( v)
    {
        prop->SetLineWidth(7.0);
        prop->SetColor(1.0,1.0,1.0);
        prop->SetOpacity(1.0);
    }   // end if
    else
    {
        prop->SetLineWidth(4.0);
        prop->SetColor(0.6,0.6,0.7);
        prop->SetOpacity(0.5);
    }   // end else
}   // end highlight

}   // end namespace

void EuclideanDistanceVisualiser::setHighlighted( const FM* fm)
{
    for ( auto& p : _actor0)
        highlight( p.second, false);
    for ( auto& p : _actor1)
        highlight( p.second, false);

    if ( fm)
    {
        for ( const FV* fv : fm->fvs())
        {
            if ( _actor0.count(fv) > 0)
                highlight( _actor0.at(fv), true);
            if ( _actor1.count(fv) > 0)
                highlight( _actor1.at(fv), true);
        }   // end for
    }   // end if
}   // end setHighlighted


bool EuclideanDistanceVisualiser::isAvailable( const FM* fm) const
{
    using SLMK = Landmark::SpecificLandmark;
    bool b0 = true;
    if ( _lmks0)
        b0 = std::all_of( std::begin(*_lmks0), std::end(*_lmks0), [fm]( const SLMK& lm){ return fm->landmarks()->has(lm);});
    bool b1 = true;
    if ( _lmks1)
        b1 = std::all_of( std::begin(*_lmks1), std::end(*_lmks1), [fm]( const SLMK& lm){ return fm->landmarks()->has(lm);});
    return b0 && b1;
}   // end isAvailable


// private
void EuclideanDistanceVisualiser::apply( FV *fv, const LmkList* ll, std::unordered_map<const FV*, vtkActor*>& actors)
{
    const FM* fm = fv->data();

    //std::cerr << "EDV::apply " << id() << std::endl;
    assert( ll);
    assert( !ll->empty());
    assert( fm->landmarks()->pos(ll->front()) != nullptr);
    assert( fm->landmarks()->pos(ll->back()) != nullptr);
    const std::vector<cv::Vec3f> vtxs = { *fm->landmarks()->pos(ll->front()), *fm->landmarks()->pos(ll->back())};

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
}   // end apply


void EuclideanDistanceVisualiser::apply( FV *fv, const QPoint*)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    clear(fv); // Ensure removed so can create new line actor(s)
    if ( _lmks0 && _lmks0->size() == 2)
        apply( fv, _lmks0, _actor0);
    if ( _lmks1 && _lmks1->size() == 2)
        apply( fv, _lmks1, _actor1);
    fm->unlock();
    MetricVisualiser::apply( fv, nullptr);
}   // end apply


// private
void EuclideanDistanceVisualiser::clear( FV *fv, std::unordered_map<const FV*, vtkActor*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end clear


void EuclideanDistanceVisualiser::clear( FV *fv)
{
    clear( fv, _actor0);
    clear( fv, _actor1);
    MetricVisualiser::clear(fv);
}   // end clear 


void EuclideanDistanceVisualiser::purge( const FM* fm)
{
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [this](FV* fv){ this->clear(fv);});
}   // end purge

