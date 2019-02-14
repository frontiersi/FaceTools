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


bool RegionVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    return ( _rep0.count(fv) > 0 && _rep0.at(fv) == p) || ( _rep1.count(fv) > 0 && _rep1.at(fv) == p);
}   // end belongs


void RegionVisualiser::pokeTransform( const FV *fv, const vtkMatrix4x4* cvm)
{
    vtkMatrix4x4* vm = const_cast<vtkMatrix4x4*>(cvm);
    if ( _rep0.count(fv) > 0)
        _rep0.at(fv)->PokeMatrix(vm);
    if ( _rep1.count(fv) > 0)
        _rep1.at(fv)->PokeMatrix(vm);
}   // end pokeTransform


void RegionVisualiser::fixTransform( const FV *fv)
{
    if ( _rep0.count(fv) > 0)
        RVTK::transform( _rep0.at(fv), _rep0.at(fv)->GetMatrix());
    if ( _rep1.count(fv) > 0)
        RVTK::transform( _rep1.at(fv), _rep1.at(fv)->GetMatrix());
}   // end fixTransform


namespace {

void highlight( vtkActor* actor, bool v)
{
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

}   // end namespace

void RegionVisualiser::setHighlighted( const FM* fm)
{
    for ( auto& p : _rep0)
        highlight( p.second, false);
    for ( auto& p : _rep1)
        highlight( p.second, false);

    if ( fm)
    {
        for ( const FV* fv : fm->fvs())
        {
            if ( _rep0.count(fv) > 0)
                highlight( _rep0.at(fv), true);
            if ( _rep1.count(fv) > 0)
                highlight( _rep1.at(fv), true);
        }   // end for
    }   // end if
}   // end setHighlighted


bool RegionVisualiser::isAvailable( const FM* fm) const
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
void RegionVisualiser::apply( FV *fv, const LmkList* lmks, std::unordered_map<const FV*, vtkActor*>& actors)
{
    const FM* fm = fv->data();
    std::vector<cv::Vec3f> vtxs;
    for ( const auto& lmk : *lmks)
        vtxs.push_back(*fm->landmarks()->pos(lmk));
    vtkActor* actor = actors[fv] = RVTK::VtkActorCreator::generateLineActor(vtxs, true);
    setActorProperties(actor);
    fv->viewer()->add( actor);
}   // end apply


void RegionVisualiser::apply( FV *fv, const QPoint*)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    clear(fv); // Ensure removed so can create new actor(s)
    if ( _lmks0 && !_lmks0->empty())
        apply( fv, _lmks0, _rep0);
    if ( _lmks1 && !_lmks1->empty())
        apply( fv, _lmks1, _rep1);
    fm->unlock();
    MetricVisualiser::apply( fv, nullptr);
}   // end apply


// private
void RegionVisualiser::clear( FV *fv, std::unordered_map<const FV*, vtkActor*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end clear


void RegionVisualiser::clear( FV *fv)
{
    clear( fv, _rep0);
    clear( fv, _rep1);
    MetricVisualiser::clear(fv);
}   // end clear 


void RegionVisualiser::purge( const FM* fm)
{
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [this](FV* fv){ this->clear(fv);});
}   // end purge

