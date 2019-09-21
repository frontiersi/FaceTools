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

#include <Interactor/RadialSelectHandler.h>
#include <Action/ModelSelector.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <MiscFunctions.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


RadialSelectHandler::RadialSelectHandler( RadialSelectVisualisation& vis, const FM* fm)
    : _vis(vis), _onReticule(false), _move(false), _model(fm), _radiusChange(0)
{
    fm->lockForRead();
    cv::Vec3f cpos = fm->findClosestSurfacePoint( fm->centre());
    const int sv = fm->findVertex( cpos);
    _rsel = RFeatures::ObjModelRegionSelector::create( fm->model(), sv);

    const double diag = fm->bounds()[0]->diagonal();
    const double initRad = diag / 3;
    _rsel->setRadius( initRad);
    _radiusChange = initRad / 100;
    fm->unlock();
}   // end ctor


void RadialSelectHandler::set( const cv::Vec3f &p, double r)    // Provided p is the transformed point
{
    _model->lockForRead();
    cv::Vec3f cpos = _model->findClosestSurfacePoint( p);
    const int sv = _model->findVertex( cpos);

#ifndef NDEBUG
    const double diff = cv::norm(p-cpos);
    std::cerr << "|VTK surface projection - model closest surface point| = " << p << " - " << cpos << " = " << diff << std::endl;
    if ( diff > 0.1)
    {
        const RFeatures::ObjModel& mod = _model->model();
        std::cerr << "  - Vertex " << sv << " at " << mod.vtx(sv) << std::endl;
        for ( int v : mod.cvtxs(sv))
            std::cerr << "    vertex " << v << " at " << mod.vtx(v) << std::endl;
        //cpos = _model->findClosestSurfacePoint( p);
    }   // end if
#endif

    _rsel->setCentre( sv, cpos);
    _rsel->setRadius(r);

    _vis.set( _model, cpos, _rsel->boundary()); // Update the visualisation (transformed point given).
    _model->unlock();

    MS::showStatus( QString( "%1  with radius %2").arg( posString( "Centre at:", cpos)).arg(r, 6, 'f', 2), 5000);
}   // end set


double RadialSelectHandler::radius() const { return _rsel->radius();}
cv::Vec3f RadialSelectHandler::centre() const { return _rsel->centre();}
size_t RadialSelectHandler::selectedFaces(IntSet &fs) const { return _rsel->selectedFaces(fs);}


void RadialSelectHandler::doEnterProp( FV* fv, const vtkProp* p)
{
    if ( _vis.belongs( p, fv))
    {
        _onReticule = true;
        _showHover( true);
    }   // end if
}   // end doEenterProp


void RadialSelectHandler::doLeaveProp( FV* fv, const vtkProp* p)
{
    if ( _vis.belongs( p, fv))
    {
        _onReticule = false;
        _showHover( false);
    }   // end if
}   // end doLeaveProp


bool RadialSelectHandler::leftButtonDown()
{
    _move = _onReticule;
    return _move;
}   // end leftButtonDown


bool RadialSelectHandler::leftButtonUp()
{
    const bool wasMoving = _move;
    _move = false;
    return wasMoving;
}   // end leftButtonUp


bool RadialSelectHandler::leftDrag()
{
    bool swallowed = false;
    FV* fv = MS::cursorView();
    if ( fv && _move)
    {
        assert( _model->fvs().has(fv));
        cv::Vec3f c;
        fv->data()->lockForRead();
        if ( fv->projectToSurface( MS::mousePos(), c))  // Gets the transformed point
        {
            MS::setCursor(Qt::CursorShape::SizeAllCursor);
            set( c, _rsel->radius());
            _showHover( true);
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
        set( _rsel->centre(), _rsel->radius() + _radiusChange);
        _showHover( true);
    }   // end if
    return _onReticule;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectHandler::mouseWheelBackward()
{
    if ( _onReticule)
    {
        set( _rsel->centre(), std::max( _rsel->radius() - _radiusChange, 0.0));
        _showHover( true);
    }   // end if
    return _onReticule;
}   // end mouseWheeBackward


void RadialSelectHandler::_showHover( bool v)
{
    if ( MS::cursorView())
    {
        if ( v)
            MS::setCursor(Qt::CursorShape::SizeAllCursor);
        else
            MS::restoreCursor();

        const FM* fm = MS::cursorView()->data();
        _vis.setHighlighted( fm, v);
        MS::updateRender();
    }   // end if
}   // end _showHover
