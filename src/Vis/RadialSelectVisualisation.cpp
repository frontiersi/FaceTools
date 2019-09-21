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

#include <Vis/RadialSelectVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <VtkTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::PointsView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::ModelViewer;
using FaceTools::Action::Event;


RadialSelectVisualisation::RadialSelectVisualisation( double srad) : _srad(srad) {}


RadialSelectVisualisation::~RadialSelectVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first), Event::NONE);
}   // end dtor


bool RadialSelectVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    bool present = false;
    if (_views.count(fv) > 0)
    {
        const View& view = *_views.at(fv);
        present = view.centre->prop() == prop;  // Only check the centre
    }   // end if
    return present;
}   // end belongs


void RadialSelectVisualisation::apply( FV* fv, const QPoint*)
{
    setVisible( fv, true);
}   // end apply


bool RadialSelectVisualisation::purge( FV* fv, Event)
{
    setVisible( fv, false);
    if ( _views.count(fv) > 0)
    {
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
    return true;
}   // end purge


void RadialSelectVisualisation::setVisible( FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool RadialSelectVisualisation::isVisible( const FV* fv) const
{
    bool vis = false;
    if ( _views.count(fv) > 0)
        vis = _views.at(fv)->visible();
    return vis;
}   // end isVisible


void RadialSelectVisualisation::syncToViewTransform( const FV *fv, const vtkMatrix4x4* d)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->pokeTransform( d);
}   // end syncToViewTransform


void RadialSelectVisualisation::set( const FM* fm, const cv::Vec3f& cpos, const IntSet* vidxs)
{
    for ( FV* fv : fm->fvs())
    {
        purge( fv, Event::NONE); // Ensures old actors are removed
        View* view = _views[fv] = new View;
        view->set( fv, cpos, vidxs, _srad);
        setVisible( fv, true);
    }   // end for
}   // end set

/*
void RadialSelectVisualisation::setPickable( const FV* fv, bool v)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->setPickable(v);
}   // end setPickable
*/

void RadialSelectVisualisation::setHighlighted( const FM* fm, bool hval)
{
    for ( FV* fv : fm->fvs())
        if ( _views.count(fv) > 0)
            _views.at(fv)->setHighlighted( hval);
}   // end setHighlighted


/*************************************************/
/******** RadialSelectVisualisation::View ********/
/*************************************************/
RadialSelectVisualisation::View::View() : centre(nullptr), points(nullptr), vidxs(nullptr) {}

RadialSelectVisualisation::View::~View() { reset();}


void RadialSelectVisualisation::View::reset()
{
    if ( centre)
    {
        centre->setVisible(false, nullptr);
        delete centre;
    }   // end if
    if ( points)
    {
        points->setVisible(false, nullptr);
        delete points;
    }   // end if
    centre = nullptr;
    points = nullptr;
    vidxs = nullptr;
}   // end reset


void RadialSelectVisualisation::View::set( const FV* fv, const cv::Vec3f& cpos, const IntSet* vds, double srad)
{
    assert(fv);

    const bool wasVisible = visible();

    // The position provided has had the model's transform matrix applied so we should actually so we should
    // use the untransformed position and update the sphere's actor transform to match the model's/view's transform matrix.
    // In this way, further poking of the actor's matrix will keep the sphere in the correct location.
    const cv::Matx44d& tmat = fv->data()->model().transformMatrix();
    const cv::Vec3f icpos = RFeatures::transform( tmat.inv(), cpos);

    if ( !centre)
    {
        centre = new SphereView( icpos, srad, true, true);
        centre->setResolution( 101);
        centre->pokeTransform( RVTK::toVTK( tmat));
    }   // end if
    else
        centre->setCentre( icpos);

    if ( vds != nullptr)
        vidxs = vds;
    assert( vidxs);

    if ( points)
    {
        points->setVisible(false,nullptr);
        delete points;
    }   // end if

    // The points view uses the model's transform matrix to set its view transform.
    // This is okay as long as setting this view always happens after the model and
    // view transforms are synchronised!
    points = new PointsView( fv->data()->model(), *vidxs, 4.0, 1,1,1, 0.99);

    setVisible( wasVisible, fv->viewer());
}   // end set


void RadialSelectVisualisation::View::setColour( double r, double g, double b, double a)
{
    if ( centre)
        centre->setColour(r,g,b,a);
    if ( points)
        points->setColour(r,g,b,a);
}   // end setColour


void RadialSelectVisualisation::View::setVisible( bool v, ModelViewer* vwr)
{
    if ( centre)
        centre->setVisible( v, vwr);
    if ( points)
        points->setVisible( v, vwr);
}   // end setVisible


bool RadialSelectVisualisation::View::visible() const { return points ? points->visible() : false;}


void RadialSelectVisualisation::View::setPickable( bool p)
{
    if ( centre)
        centre->setPickable(p);
}   // end setPickable


void RadialSelectVisualisation::View::pokeTransform( const vtkMatrix4x4* d)
{
    if ( centre)
        centre->pokeTransform( d);
    if ( points)
        points->pokeTransform(d);
}   // end pokeTransform


void RadialSelectVisualisation::View::setHighlighted( bool hval)
{
    const cv::Vec3d col = hval ? cv::Vec3d(0.9, 0.2, 0.4) : cv::Vec3d(0.0, 0.6, 0.0);
    setColour( col[0], col[1], col[2], 0.5);
}   // end setHighlighted
/*************************************************/
/*************************************************/
/*************************************************/
