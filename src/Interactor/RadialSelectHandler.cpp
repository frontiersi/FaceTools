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


void RadialSelectHandler::init( const FM *fm, const Vec3f &tpos, float r)    // Provided p is the transformed point
{
    fm->lockForRead();
    _rsel = r3d::RegionSelector::create( fm->mesh());
    _update( tpos, r);
    fm->unlock();
}   // end init


void RadialSelectHandler::refreshState()
{
    if ( !_isRegionSelectorForModel(MS::selectedModel()))
    {
        _fids.clear();
        _bnds.reset();
        _rsel = nullptr;
    }   // end if
    setEnabled( MS::isViewSelected() && MS::selectedView()->isApplied(&_vis));
}   // end refreshState


float RadialSelectHandler::radius() const { return _rsel ? _rsel->radius() : 0.0f;}
Vec3f RadialSelectHandler::centre() const { return _rsel ? _rsel->centre() : Vec3f::Zero();}


const std::list<int> &RadialSelectHandler::boundaryVertices() const
{
    static const std::list<int> EMPTY_LIST;
    return _bnds.count() > 0 ? _bnds.boundary(0) : EMPTY_LIST;
}   // end boundaryVertices


void RadialSelectHandler::_update( Vec3f tpos, float r)
{
    const FM *fm = MS::selectedModel();
    assert(fm);
    const r3d::Mesh &mesh = fm->mesh();
    assert( &_rsel->mesh() == &mesh);

    _radiusChange = r / 30;

    const r3d::Manifolds &manifolds = fm->manifolds();  // Only extract from a single manifold

    if ( _lmkVis && _lmkVis->isVisible( MS::selectedView()))
    {
        const float snapDist = 0.015f * (MS::selectedViewer()->camera().pos() - tpos).norm();
        tpos = fm->currentLandmarks().snapToVisible( tpos, snapDist*snapDist);
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

    _vis.refreshState(MS::selectedView());
    _showHover();
    MS::showStatus( QString( "%1  with radius %2 %3").arg( posString( "Centre at:", tpos)).arg(r, 6, 'f', 2).arg(FM::LENGTH_UNITS), 5000);
    MS::updateRender();
}   // end _update


bool RadialSelectHandler::_isRegionSelectorForModel( const FM *fm) const
{
    return _rsel && fm && &fm->mesh() == &_rsel->mesh();
}   // end _isRegionSelectorForModel


void RadialSelectHandler::doEnterProp( FV* fv, const vtkProp* p)
{
    if ( _isRegionSelectorForModel(fv->data()) && _vis.belongs( p, fv))
    {
        _onReticule = true;
        _showHover();
    }   // end if
}   // end doEnterProp


void RadialSelectHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    if ( _isRegionSelectorForModel(fv->data()) && _vis.belongs( p, fv))
    {
        _onReticule = false;
        _showHover();
    }   // end if
}   // end doLeaveProp


bool RadialSelectHandler::leftButtonDown() { return (_moving = _onReticule);}


bool RadialSelectHandler::leftButtonUp()
{
    const bool wasMoving = _moving;
    _moving = false;
    return wasMoving;
}   // end leftButtonUp


bool RadialSelectHandler::leftDrag()
{
    bool swallowed = false;
    if ( _moving)
    {
        const FV *fv = MS::selectedView();
        fv->data()->lockForRead();
        Vec3f c;
        if ( fv->projectToSurface( fv->viewer()->mouseCoords(), c))  // Gets the transformed point
        {
            _update( c, _rsel->radius());
            swallowed = true;
        }   // end if
        fv->data()->unlock();
    }   // end if
    return swallowed;
}   // end leftDrag


// Increase the radius
bool RadialSelectHandler::mouseWheelForward()
{
    if ( _onReticule)
    {
        assert(_rsel);
        const FM *fm = MS::selectedModel();
        fm->lockForRead();
        _update( _rsel->centre(), _rsel->radius() + _radiusChange);
        fm->unlock();
    }   // end if
    return _onReticule;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectHandler::mouseWheelBackward()
{
    if ( _onReticule)
    {
        const FM *fm = MS::selectedModel();
        fm->lockForRead();
        _update( _rsel->centre(), std::max( _rsel->radius() - _radiusChange, 0.0f));
        fm->unlock();
    }   // end if
    return _onReticule;
}   // end mouseWheeBackward


void RadialSelectHandler::_showHover()
{
    if ( _onReticule || _moving)
        MS::setCursor( Qt::CursorShape::SizeAllCursor);
    else
        MS::restoreCursor();
    _vis.setHighlighted( MS::selectedView(), _onReticule);
}   // end _showHover
