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

#include <Interactor/RadialSelectHandler.h>
#include <Interactor/LandmarksHandler.h>
#include <Action/ModelSelector.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::FV;
using FaceTools::Vec3f;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


RadialSelectHandler::Ptr RadialSelectHandler::create() { return Ptr( new RadialSelectHandler);}


// private
RadialSelectHandler::RadialSelectHandler()
    : _onReticule(false), _moving(false), _radiusChange(0), _lmkVis(nullptr)
{
    _vis.setHandler(this);
}   // end ctor


void RadialSelectHandler::postRegister()
{
    _lmkVis = &MS::handler<LandmarksHandler>()->visualisation();
    assert(_lmkVis);
}   // end postRegister


// Provided p is the transformed point
void RadialSelectHandler::init( const FM *fm, const Vec3f &tpos, float r)
{
    fm->lockForRead();
    _rsel = r3d::RegionSelector::create( fm->mesh());
    _update( tpos, r);
    fm->unlock();
}   // end init


void RadialSelectHandler::refresh()
{
    const FV *fv = MS::selectedView();
    setEnabled( fv && _vis.isVisible(fv));
    if ( fv && !_isRegionSelectorForModel( fv->data()))
    {
        _fids.clear();
        _bnds.reset();
        _rsel = nullptr;
    }   // end if
}   // end refresh


float RadialSelectHandler::radius() const { return _rsel ? _rsel->radius() : 0.0f;}
Vec3f RadialSelectHandler::centre() const { return _rsel ? _rsel->centre() : Vec3f::Zero();}


const std::list<int> &RadialSelectHandler::boundaryVertices() const
{
    static const std::list<int> EMPTY_LIST;
    return _bnds.count() > 0 ? _bnds.boundary(0) : EMPTY_LIST;
}   // end boundaryVertices


void RadialSelectHandler::_update( Vec3f tpos, float r)
{
    const FV *fv = MS::selectedView();
    const FM *fm = fv->data();
    assert(fm);
    const r3d::Mesh &mesh = fm->mesh();
    assert( &_rsel->mesh() == &mesh);

    _radiusChange = r / 30;

    const r3d::Manifolds &manifolds = fm->manifolds();  // Only extract from a single manifold

    if ( _lmkVis->isVisible( fv))
    {
        const float snapDist = 0.015f * (fv->viewer()->camera().pos() - tpos).norm();
        tpos = fm->currentLandmarks().snapTo( tpos, snapDist*snapDist);
    }   // end if

    const int sv = fm->findVertex(tpos);
    // Get the manifold having this vertex
    const r3d::Manifold& man = manifolds[manifolds.fromFaceId( *mesh.faces(sv).begin())];

    _rsel->update( sv, tpos, r);
    _fids.clear();
    _rsel->selectedFaces( _fids, &man.faces());
    // If there are no selected faces, get the face that the centre point is inside
    if ( _fids.empty())
    {
        for ( int fid : mesh.faces(sv))
        {
            const Vec3f ppos = mesh.projectToFacePlane(fid, tpos);
            if ( mesh.isVertexInsideFace( fid, ppos))
                _fids.insert(fid);
        }   // end for
    }   // end if

    _bnds.reset();
    _bnds.sort( mesh, mesh.pseudoBoundaries( _fids));

    _vis.refresh(fv);
    _showHover();
    MS::showStatus( QString( "%1  with radius %2 %3").arg( posString( "Centre at:", tpos)).arg(r, 6, 'f', 2).arg(FM::LENGTH_UNITS), 5000);
    // Update across all viewers 
    if ( fm->fvs().size() > 1)
        MS::updateRender();
}   // end _update


bool RadialSelectHandler::_isRegionSelectorForModel( const FM *fm) const
{
    return _rsel && fm && &fm->mesh() == &_rsel->mesh();
}   // end _isRegionSelectorForModel


bool RadialSelectHandler::_testInProp( bool onRet)
{
    bool swallowed = false;
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    if ( _isRegionSelectorForModel( fv->data()) && _vis.belongs( p, fv))
    {
        swallowed = true;
        _onReticule = onRet;
        _showHover();
    }   // end if
    return swallowed;
}   // end _testInProp


bool RadialSelectHandler::doEnterProp() { return _testInProp( true);}
bool RadialSelectHandler::doLeaveProp() { return _testInProp( false);}


bool RadialSelectHandler::doLeftButtonDown()
{
    bool swallowed = false;
    if ( _onReticule)
    {
        swallowed = true;
        _moving = _onReticule;
    }   // end if
    return swallowed;
}   // end doLeftButtonDown


bool RadialSelectHandler::doLeftButtonUp()
{
    bool swallowed = false;
    if ( _moving)
    {
        swallowed = true;
        _moving = false;
    }   // end if
    return swallowed;
}   // end doLeftButtonUp


bool RadialSelectHandler::doLeftDrag()
{
    bool swallowed = false;
    if ( _moving)
    {
        swallowed = true;
        const FV *fv = MS::selectedView();
        const FM *fm = fv->data();
        fm->lockForRead();
        Vec3f c;
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), c))  // Gets the transformed point
            _update( c, _rsel->radius());
        fm->unlock();
    }   // end if
    return swallowed;
}   // end doLeftDrag


bool RadialSelectHandler::_changeRadius( float rchng)
{
    bool swallowed = false;
    if ( _onReticule)
    {
        swallowed = true;
        const FV *fv = MS::selectedView();
        fv->data()->lockForRead();
        _update( _rsel->centre(), std::max( _rsel->radius() + rchng, 0.0f));
        fv->data()->unlock();
    }   // end if
    return swallowed;
}   // end _changeRadius


bool RadialSelectHandler::doMouseWheelForward() { return _changeRadius( +_radiusChange);}
bool RadialSelectHandler::doMouseWheelBackward() { return _changeRadius( -_radiusChange);}


void RadialSelectHandler::_showHover()
{
    if ( _onReticule || _moving)
        MS::setCursor( Qt::CursorShape::SizeAllCursor);
    else
        MS::restoreCursor();
    _vis.setHighlighted( MS::selectedView(), _onReticule);
}   // end _showHover
