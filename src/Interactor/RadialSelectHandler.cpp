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

#include <Interactor/RadialSelectHandler.h>
#include <Interactor/LandmarksHandler.h>
#include <ModelSelect.h>
#include <MiscFunctions.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::FV;
using FaceTools::Vec3f;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


RadialSelectHandler::Ptr RadialSelectHandler::create() { return Ptr( new RadialSelectHandler);}


// private
RadialSelectHandler::RadialSelectHandler()
    : _fm(nullptr), _onReticule(false), _moving(false), _radiusChange(0)
{
    _vis.setHandler(this);
}   // end ctor


// Provided p is the transformed point
void RadialSelectHandler::init( const FM *fm, const Vec3f &tpos, float r)
{
    fm->lockForRead();
    _fm = fm;
    _rsel = r3d::RegionSelector::create( fm->mesh());
    _update( tpos, r);
    fm->unlock();
    refresh();
}   // end init


void RadialSelectHandler::reset( const FM *fm)
{
    if ( _fm != fm)
        return;
    _fm = nullptr;
    _onReticule = false;
    _moving = false;
    _radiusChange = 0;
    _fids.clear();
    _bnds.reset();
    _rsel = nullptr;
    refresh();
}   // end reset


void RadialSelectHandler::refresh()
{
    const FV *fv = MS::selectedView();
    if ( !fv || fv->data() != _fm)
    {
        _fids.clear();
        _bnds.reset();
        _rsel = nullptr;
        _vis.purgeAll();
    }   // end if
    setEnabled( !!_rsel);   // Teehee ;D
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
    const r3d::Mesh &mesh = fm->mesh();
    assert( &_rsel->mesh() == &mesh);

    _radiusChange = r / 30;

    const r3d::Manifolds &manifolds = fm->manifolds();  // Only extract from a single manifold

    const Vis::BaseVisualisation *lmkVis = &MS::handler<LandmarksHandler>()->visualisation();
    if ( lmkVis->isVisible( fv))
    {
        const float snapDist = 0.015f * (fv->viewer()->camera().pos() - tpos).norm();
        tpos = fm->currentLandmarks().snapTo( tpos, snapDist*snapDist);
    }   // end if

    // Find the vertex and face tpos is at on the mesh
    const int sv = fm->findVertex(tpos);
    int cfid = -1;
    for ( int fid : mesh.faces(sv))
    {
        const Vec3f ppos = mesh.projectToFacePlane(fid, tpos);
        if ( mesh.isVertexInsideFace( fid, ppos))
        {
            cfid = fid;
            break;
        }   // end if
    }   // end for
    if ( cfid < 0)
        cfid = *mesh.faces(sv).begin();

    // Get the manifold having this face
    const r3d::Manifold& man = manifolds[manifolds.fromFaceId( cfid)];

    _rsel->update( cfid, tpos, r);
    _fids.clear();
    _rsel->selectedFaces( _fids, &man.faces());
    _bnds.reset();
    _bnds.sort( mesh, mesh.pseudoBoundaries( _fids));

    _showHover();
    MS::showStatus( QString( "%1  with radius %2 %3").arg( posString( "Centre at:", tpos)).arg(r, 6, 'f', 2).arg(FM::LENGTH_UNITS), 5000);
    // Update across all viewers 
    _vis.update(fm);
    MS::updateRender();
}   // end _update


bool RadialSelectHandler::_testInProp( bool onRet)
{
    bool swallowed = false;
    const FV *fv = MS::selectedView();
    const vtkProp *p = this->prop();
    if (( fv->data() == _fm) && _vis.belongs( p, fv))
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
        FM::RPtr fm = fv->rdata();
        Vec3f c;
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), c))  // Gets the transformed point
            _update( c, _rsel->radius());
    }   // end if
    return swallowed;
}   // end doLeftDrag


bool RadialSelectHandler::_changeRadius( float rchng)
{
    bool swallowed = false;
    if ( _onReticule)
    {
        swallowed = true;
        FM::RPtr fm = MS::selectedModelScopedRead();
        _update( _rsel->centre(), std::max( _rsel->radius() + rchng, 0.0f));
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
