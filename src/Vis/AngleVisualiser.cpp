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

#include <AngleVisualiser.h>
#include <LandmarksManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>           // RVTK
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <vtkFollower.h>
using FaceTools::Vis::AngleVisualiser;
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::FaceLateral;


AngleVisualiser::AngleVisualiser( int id, int l0, int lc, int l1, bool b)
    : MetricVisualiser( id), _l0(l0), _lc(lc), _l1(l1), _bilat(b)
{
}   // end ctor


namespace {
bool isProp( vtkAngleRepresentation3D* rp, const vtkProp* p)
{
    return rp->GetRay1() == p || rp->GetRay2() == p || rp->GetArc() == p;
}   // end isProp

void pokeMatrix( vtkAngleRepresentation3D* rp, vtkMatrix4x4* vm)
{
    rp->GetRay1()->PokeMatrix(vm);
    rp->GetRay2()->PokeMatrix(vm);
    rp->GetArc()->PokeMatrix(vm);
    rp->GetTextActor()->PokeMatrix(vm);
}   // end pokeMatrix


void fixMatrix( vtkAngleRepresentation3D* rp)
{
    RVTK::transform( rp->GetRay1(), rp->GetRay1()->GetMatrix());
    RVTK::transform( rp->GetRay2(), rp->GetRay2()->GetMatrix());
    RVTK::transform( rp->GetArc(), rp->GetArc()->GetMatrix());
    RVTK::transform( rp->GetTextActor(), rp->GetTextActor()->GetMatrix());
}   // end fixMatrix

}   // end namespace


bool AngleVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _angle0.count(fv) > 0 && isProp( _angle0.at(fv), p))
        return true;
    else if ( _angle1.count(fv) > 0 && isProp( _angle1.at(fv), p))
        return true;
    return false;
}   // end belongs


void AngleVisualiser::pokeTransform( const FV *fv, const vtkMatrix4x4* cvm)
{
    vtkMatrix4x4* vm = const_cast<vtkMatrix4x4*>(cvm);
    if ( _angle0.count(fv) > 0)
        pokeMatrix( _angle0.at(fv), vm);
    if ( _angle1.count(fv) > 0)
        pokeMatrix( _angle1.at(fv), vm);
}   // end pokeTransform


void AngleVisualiser::fixTransform( const FV *fv)
{
    if ( _angle0.count(fv) > 0)
        fixMatrix( _angle0.at(fv));
    if ( _angle1.count(fv) > 0)
        fixMatrix( _angle1.at(fv));
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


void highlight( vtkAngleRepresentation3D* rp, bool v)
{
    highlight( rp->GetRay1(), v);
    highlight( rp->GetRay2(), v);
    highlight( rp->GetArc(), v);
    highlight( rp->GetTextActor(), v);
}   // end highlight


void setActorProperties( vtkActor* actor)
{
    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetAmbient( 1);
    property->SetDiffuse( 0);
    property->SetSpecular(0);
}   // end setActorProperties


void setAngleProperties( vtkAngleRepresentation3D* rp)
{
    setActorProperties( rp->GetRay1());
    setActorProperties( rp->GetRay2());
    setActorProperties( rp->GetArc());
    setActorProperties( rp->GetTextActor());
    highlight( rp, false);
    rp->SetPickable(false);
}   // end setAngleProperties

}   // end namespace

void AngleVisualiser::setHighlighted( const FM* fm)
{
    for ( auto& p : _angle0)
        highlight( p.second, false);

    for ( auto& p : _angle1)
        highlight( p.second, false);

    if ( fm)
    {
        for ( const FV* fv : fm->fvs())
            if ( _angle0.count(fv) > 0)
                highlight( _angle0.at(fv), true);
        if ( _bilat)
        {
            for ( const FV* fv : fm->fvs())
                if ( _angle1.count(fv) > 0)
                    highlight( _angle1.at(fv), true);
        }   // end if
    }   // end if
}   // end setHighlighted


// private
void AngleVisualiser::apply( FV *fv, FaceLateral lA, FaceLateral lB, FaceLateral lC,
                                     std::unordered_map<const FV*, vtkAngleRepresentation3D*>& angles)
{
    const FM* fm = fv->data();
    cv::Vec3d r0 = *fm->landmarks()->pos(_l0, lA);
    cv::Vec3d r1 = *fm->landmarks()->pos(_l1, lC);
    cv::Vec3d cp;
    // Set the centre point according to the given landmark, but if it's < 0,
    // the centre point is set as the point on the surface midway between the endpoints.
    if ( _lc >= 0)
        cp = *fm->landmarks()->pos(_lc, lB);
    else
    {
        const cv::Vec3f mp( 0.5 * (r0 + r1));
        cp = FaceTools::toSurface( fm->kdtree(), mp);
    }   // end else

    vtkAngleRepresentation3D* angle = angles[fv] = vtkAngleRepresentation3D::New();
    angle->SetPoint1WorldPosition(&r0[0]);
    angle->SetCenterWorldPosition(&cp[0]);
    angle->SetPoint2WorldPosition(&r1[0]);

    setAngleProperties( angle);
    fv->viewer()->add( angle);
}   // end apply


void AngleVisualiser::apply( FV *fv, const QPoint*)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    clear(fv); // Ensure removed so can create new actor(s)

    if ( _bilat)
    {
        FaceLateral latA0 = FACE_LATERAL_LEFT;
        FaceLateral latB0 = FACE_LATERAL_LEFT;
        FaceLateral latC0 = FACE_LATERAL_LEFT;
        FaceLateral latA1 = FACE_LATERAL_RIGHT;
        FaceLateral latB1 = FACE_LATERAL_RIGHT;
        FaceLateral latC1 = FACE_LATERAL_RIGHT;

        assert( LDMKS_MAN::landmark(_l0)->isBilateral());
        assert( LDMKS_MAN::landmark(_l1)->isBilateral());

        if ( (_lc >= 0) && !LDMKS_MAN::landmark(_lc)->isBilateral())
        {
            latB0 = FACE_LATERAL_MEDIAL;
            latB1 = FACE_LATERAL_MEDIAL;
        }   // end else if

        apply( fv, latA0, latB0, latC0, _angle0);
        apply( fv, latA1, latB1, latC1, _angle1);
    }   // end if
    else
    {
        // Not a bilateral measure, which means that either the landmarks are the same, or that
        // both of them are not bilateral landmarks.
        if ( _l0 != _l1)    // Not bilateral, but different landmarks, so must be medial landmarks
            apply( fv, FACE_LATERAL_MEDIAL, FACE_LATERAL_MEDIAL, FACE_LATERAL_MEDIAL, _angle0);
        else
            apply( fv, FACE_LATERAL_LEFT, FACE_LATERAL_MEDIAL, FACE_LATERAL_RIGHT, _angle0); // Same landmark - one actor
    }   // end else

    fm->unlock();
    MetricVisualiser::apply( fv, nullptr);
}   // end apply


// private
void AngleVisualiser::clear( FV *fv, std::unordered_map<const FV*, vtkAngleRepresentation3D*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end clear


void AngleVisualiser::clear( FV *fv)
{
    clear( fv, _angle0);
    clear( fv, _angle1);
    MetricVisualiser::clear(fv);
}   // end clear 


void AngleVisualiser::purge( const FM* fm)
{
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [this](FV* fv){ this->clear(fv);});
}   // end purge

