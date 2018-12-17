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


EuclideanDistanceVisualiser::EuclideanDistanceVisualiser( int id, int lA, int lB, bool b)
    : MetricVisualiser( id), _lmkA(lA), _lmkB(lB), _bilat(b)
{
#ifndef NDEBUG
    if ( _bilat)
        assert( (lA != lB) && (LDMKS_MAN::landmark(lA)->isBilateral() || LDMKS_MAN::landmark(lB)->isBilateral()));
#endif
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
            if ( _actor0.count(fv) > 0)
                highlight( _actor0.at(fv), true);
        if ( _bilat)
        {
            for ( const FV* fv : fm->fvs())
                if ( _actor1.count(fv) > 0)
                    highlight( _actor1.at(fv), true);
        }   // end if
    }   // end if
}   // end setHighlighted


// private
void EuclideanDistanceVisualiser::apply( FV *fv, FaceLateral lA, FaceLateral lB,
                                         std::unordered_map<const FV*, vtkActor*>& actors)
{
    const FM* fm = fv->data();
    const std::vector<cv::Vec3f> vtxs = { *fm->landmarks()->pos(_lmkA, lA), *fm->landmarks()->pos(_lmkB, lB)};

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

    if ( _bilat)
    {
        FaceLateral latA0 = FACE_LATERAL_LEFT;
        FaceLateral latB0 = FACE_LATERAL_LEFT;
        FaceLateral latA1 = FACE_LATERAL_RIGHT;
        FaceLateral latB1 = FACE_LATERAL_RIGHT;

        // Check if either of the landmarks AREN'T bilateral. Note that they can't BOTH
        // be non-bilateral, or this visualisation wouldn't be bilateral in the first place.
        if ( !LDMKS_MAN::landmark(_lmkA)->isBilateral())
        {
            latA0 = FACE_LATERAL_MEDIAL;
            latA1 = FACE_LATERAL_MEDIAL;
        }   // end if
        else if ( !LDMKS_MAN::landmark(_lmkB)->isBilateral())
        {
            latB0 = FACE_LATERAL_MEDIAL;
            latB1 = FACE_LATERAL_MEDIAL;
        }   // end else if

        apply( fv, latA0, latB0, _actor0);
        apply( fv, latA1, latB1, _actor1);
    }   // end if
    else
    {
        // Not a bilateral measure, which means that either the landmarks are the same, or that
        // both of them are not bilateral landmarks.
        if ( _lmkA != _lmkB)    // Not bilateral, but different landmarks, so must be medial landmarks
            apply( fv, FACE_LATERAL_MEDIAL, FACE_LATERAL_MEDIAL, _actor0);
        else
            apply( fv, FACE_LATERAL_LEFT, FACE_LATERAL_RIGHT, _actor0); // Same landmark - one actor
    }   // end else

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

