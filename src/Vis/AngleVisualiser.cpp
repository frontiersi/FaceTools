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

#include <Vis/AngleVisualiser.h>
#include <LndMrk/LandmarksManager.h>
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
using FaceTools::Landmark::LmkList;


AngleVisualiser::AngleVisualiser( int id, const LmkList* l0, const LmkList* l1)
    : MetricVisualiser( id), _lmks0(l0), _lmks1(l1)
{
}   // end ctor


namespace {
bool isProp( vtkAngleRepresentation3D* rp, const vtkProp* p)
{
    return rp->GetRay1() == p || rp->GetRay2() == p || rp->GetArc() == p;
}   // end isProp

void pokeMatrix( vtkAngleRepresentation3D* rp, const vtkMatrix4x4* cvm)
{
    vtkMatrix4x4* vm = const_cast<vtkMatrix4x4*>(cvm);
    rp->GetRay1()->PokeMatrix(vm);
    rp->GetRay2()->PokeMatrix(vm);
    rp->GetArc()->PokeMatrix(vm);
    rp->GetTextActor()->PokeMatrix(vm);
}   // end pokeMatrix


void highlight( vtkActor* actor, bool v)
{
    assert(actor);
    vtkProperty* prop = actor->GetProperty();
    if ( v)
    {
        prop->SetLineWidth(7.0);
        prop->SetColor(1.0,1.0,1.0);
        prop->SetOpacity(0.99);
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


bool AngleVisualiser::belongs( const vtkProp* p, const FV *fv) const
{
    if ( _angle0.count(fv) > 0 && isProp( _angle0.at(fv), p))
        return true;
    else if ( _angle1.count(fv) > 0 && isProp( _angle1.at(fv), p))
        return true;
    return false;
}   // end belongs


void AngleVisualiser::syncToViewTransform( const FV* fv, const vtkMatrix4x4* vm)
{
    if ( _angle0.count(fv) > 0)
        pokeMatrix( _angle0.at(fv), vm);
    if ( _angle1.count(fv) > 0)
        pokeMatrix( _angle1.at(fv), vm);
}   // end syncToViewTransform


void AngleVisualiser::setHighlighted( const FV* fv, bool v)
{
    if ( _angle0.count(fv) > 0)
        highlight( _angle0.at(fv), v);
    if ( _angle1.count(fv) > 0)
        highlight( _angle1.at(fv), v);
}   // end setHighlighted


void AngleVisualiser::doSetVisible( const FV* fv, bool v)
{
    if ( _angle0.count(fv) > 0)
        _angle0.at(fv)->SetVisibility(v);
    if ( _angle1.count(fv) > 0)
        _angle1.at(fv)->SetVisibility(v);
}   // end doSetVisible


bool AngleVisualiser::isVisible( const FV *fv) const
{
    bool vis = false;
    if ( _angle0.count(fv) > 0)
        vis = _angle0.at(fv)->GetVisibility() != 0;
    if ( _angle1.count(fv) > 0)
        vis &= _angle1.at(fv)->GetVisibility() != 0;
    return vis;
}   // end visible


bool AngleVisualiser::isAvailable( const FM* fm) const
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


void AngleVisualiser::doApply( const FV *fv)
{
    if ( _lmks0 && !_lmks0->empty())
        applyActor( fv, _lmks0, _angle0);
    if ( _lmks1 && !_lmks1->empty())
        applyActor( fv, _lmks1, _angle1);
}   // end doApply


void AngleVisualiser::doPurge( const FV *fv)
{
    purgeActor( fv, _angle0);
    purgeActor( fv, _angle1);
}   // end hide


// private
void AngleVisualiser::applyActor( const FV *fv, const LmkList* ll, std::unordered_map<const FV*, vtkAngleRepresentation3D*>& angles)
{
    const FM* fm = fv->data();
    const Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    cv::Vec3d r0 = lmks.pos( ll->front());
    cv::Vec3d r1 = lmks.pos( ll->back());
    cv::Vec3d cp;
    // Set the centre point according to the given landmark, but if it's < 0,
    // the centre point is set as the point on the surface midway between the endpoints.
    if ( ll->size() == 3)
        cp = lmks.pos( ll->at(1));
    else
    {
        const cv::Vec3f mp( 0.5 * (r0 + r1));
        cp = FaceTools::toSurface( fm, mp);
    }   // end else

    vtkAngleRepresentation3D* angle = angles[fv] = vtkAngleRepresentation3D::New();
    angle->SetPoint1WorldPosition(&r0[0]);
    angle->SetCenterWorldPosition(&cp[0]);
    angle->SetPoint2WorldPosition(&r1[0]);

    setAngleProperties( angle);
    fv->viewer()->add( angle);
}   // end applyActor


// private
void AngleVisualiser::purgeActor( const FV *fv, std::unordered_map<const FV*, vtkAngleRepresentation3D*>& actors)
{
    if ( actors.count(fv) > 0)
    {
        FMV* viewer = fv->viewer();
        viewer->remove( actors.at(fv));
        actors.at(fv)->Delete();
        actors.erase(fv);
    }   // end if
}   // end purgeActor
