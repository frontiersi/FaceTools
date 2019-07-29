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

#include <RadialSelectInteractor.h>
#include <LoopSelectVisualisation.h>
#include <FaceModelViewer.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::FaceViewInteractor;
using FaceTools::Vis::LoopSelectVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


RadialSelectInteractor::RadialSelectInteractor( LoopSelectVisualisation& vis, const FM* fm)
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


void RadialSelectInteractor::set( const cv::Vec3f &p, double r)
{
    _model->lockForRead();
    cv::Vec3f cpos = _model->findClosestSurfacePoint( p);
    const int sv = _model->findVertex( cpos);
    cv::Vec3f offset = cpos - _model->model().vtx(sv); // Offset from the vertex
    _rsel->setCentre( sv, offset);
    _rsel->setRadius(r);
    updateVis();
    _model->unlock();
}   // end set


// private
void RadialSelectInteractor::updateVis()
{
    // Get the boundary as a vector of vertices
    const IntSet* vidxs = _rsel->boundary();
    std::vector<cv::Vec3f> pts;
    const RFeatures::ObjModel& cmodel = _model->model();
    std::for_each( std::begin(*vidxs), std::end(*vidxs), [&](int v){ pts.push_back(cmodel.vtx(v));});

    cv::Vec3f v = _rsel->centre(); // Get the centre vertex

    // Update the visualisation
    for ( const FV* fv : _model->fvs())
    {
        _vis.setReticule( fv, v);
        _vis.setPoints( fv, pts);
    }   // end for

    MS::updateRender();
}   // end updateVis


double RadialSelectInteractor::radius() const { return _rsel->radius();}
cv::Vec3f RadialSelectInteractor::centre() const { return _rsel->centre();}
size_t RadialSelectInteractor::selectedFaces(IntSet &fs) const { return _rsel->selectedFaces(fs);}


void RadialSelectInteractor::enterProp( FV* fv, const vtkProp* p)
{
    if ( _vis.belongs( p, fv))
    {
        _onReticule = true;
        showHover( true);
    }   // end if
}   // end enterProp


void RadialSelectInteractor::leaveProp( FV* fv, const vtkProp* p)
{
    if ( _vis.belongs( p, fv))
    {
        _onReticule = false;
        showHover( false);
    }   // end if
}   // end leaveProp


bool RadialSelectInteractor::leftButtonDown()
{
    _move = _onReticule;
    return _move;
}   // end leftButtonDown


bool RadialSelectInteractor::leftButtonUp()
{
    const bool wasMoving = _move;
    _move = false;
    return wasMoving;
}   // end leftButtonUp


bool RadialSelectInteractor::leftDrag()
{
    FaceViewInteractor::leftDrag();
    bool swallowed = false;
    FV* fv = view();
    if ( fv && _move)
    {
        assert( _model->fvs().has(fv));
        cv::Vec3f c;
        fv->data()->lockForRead();
        if ( fv->projectToSurface( MS::mousePos(), c))
        {
            MS::setCursor(Qt::CursorShape::SizeAllCursor);
            set( c, _rsel->radius());
            swallowed = true;
        }   // end if
        fv->data()->unlock();
    }   // end if
    return swallowed;
}   // end leftDrag


// Increase the radius
bool RadialSelectInteractor::mouseWheelForward()
{
    if ( _onReticule)
        set( _rsel->centre(), _rsel->radius() + _radiusChange);
    return _onReticule;
}   // end mouseWheelForward


// Decrease the radius
bool RadialSelectInteractor::mouseWheelBackward()
{
    if ( _onReticule)
        set( _rsel->centre(), std::max( _rsel->radius() - _radiusChange, 0.0));
    return _onReticule;
}   // end mouseWheeBackward


void RadialSelectInteractor::showHover( bool v)
{
    if ( view())
    {
        if ( v)
            MS::setCursor(Qt::CursorShape::SizeAllCursor);
        else
            MS::restoreCursor();

        const FM* fm = view()->data();
        for ( FV* f : fm->fvs())
            _vis.setHighlighted( f, v);

        MS::updateRender();
    }   // end if
}   // end showHover
