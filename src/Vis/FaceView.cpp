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

#include <FaceView.h>
#include <FaceModel.h>
#include <FaceModelViewer.h>
#include <SurfaceMetricsMapper.h>
#include <BaseVisualisation.h>
#include <MetricVisualiser.h>
#include <vtkProperty.h>
#include <vtkCellData.h>
#include <VtkTools.h>   // RVTK::transform
#include <QColor>
#include <iostream>
#include <cassert>
using FaceTools::Vis::FaceView;
using BV = FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::SurfaceMetricsMapper;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Action::Event;
using FaceTools::Action::EventGroup;


FaceView::FaceView( FM* fm, FMV* viewer)
    : _data(fm), _actor(nullptr), _texture(nullptr), _viewer(nullptr), _pviewer(nullptr),
      _smm(nullptr), _baseCol(200,190,210), _xvis(nullptr), _nMetricLayers(0)
{
    assert(viewer);
    assert(fm);
    _data->addView(this);
    setViewer(viewer);
    reset();
}   // end ctor


bool FaceView::copyFrom( const FaceView* fv)
{
    assert( data() == fv->data());
    if ( data() != fv->data())
        return false;

    assert( _actor != fv->_actor);
    assert( _viewer != fv->_viewer);

    setBackfaceCulling(fv->backfaceCulling());
    setOpacity(fv->opacity());
    setColour(fv->colour());

    for ( BaseVisualisation* vl : fv->visualisations())
    {
        assert( vl->isAvailable(data()));
        if ( vl->isVisible(fv))
            apply(vl);
    }   // end for

    _updateModelLighting();

    return true;
}   // end copyFrom


FaceView::~FaceView()
{
    while ( !_vlayers.empty())
        purge( *_vlayers.begin(), Event::NONE);
    setViewer(nullptr);
    _data->eraseView(this);
    if ( _actor)
        _actor->Delete();
}   // end dtor


void FaceView::setViewer( FMV* viewer)
{
    // Record which visualisations are hidden on the current viewer
    // (these ones won't be applied in the new viewer).
    VisualisationLayers wasHidden;

    if ( _viewer)
    {
        _viewer->detach(this);
        for ( BV* vis : _vlayers)
        {
            if ( !vis->isVisible(this))
                wasHidden.insert(vis);
            vis->setVisible(this,false);
        }   // end for
        _viewer->remove(_actor);
    }   // end if

    _pviewer = _viewer;
    _viewer = viewer;

    if ( _viewer)
    {
        _viewer->add(_actor);
        for ( BV* vis : _vlayers)
        {
            if ( wasHidden.count(vis) == 0)    // Apply if wasn't hidden on the previous viewer
                vis->apply(this);
        }   // end for
        _viewer->attach(this);
    }   // end if
}   // end setViewer


void FaceView::reset()
{
    assert(_viewer);

    const bool bface = backfaceCulling();
    const bool tex = textured();
    const bool wf = wireframe();
    const double op = opacity();
    const QColor cl = colour();

    if ( _actor)
    {
        _viewer->remove(_actor);    // Remove the actor
        _actor->Delete();
        _actor = nullptr;
    }   // end if

    // Create the new actor from the data
    _actor = RVTK::VtkActorCreator::generateActor( _data->model(), _texture);

    setBackfaceCulling(bface);
    setTextured(tex);
    setWireframe(wf);
    setOpacity(op);
    setColour(cl);

    _viewer->add(_actor);   // Re-add the newly generated actor

    // Re-apply the old visualisation layers - now unavailable ones or ones
    // that are not visible are left unapplied (the non-visible ones will be
    // applied later when/if necessary by their parent ActionVisualise objects).
    auto vlayers = _vlayers;
    _xvis = nullptr;
    _nMetricLayers = 0;
    for ( BV* vis : vlayers)
    {
        const bool wasVisible = vis->isVisible(this);
        purge(vis, Event::NONE);

        if ( vis->isAvailable(_data))
        {
            if ( wasVisible)
                this->apply(vis);
        }   // end if
    }   // end for
}   // end reset


bool FaceView::purge( BV* vis, Event e)
{
    assert(_viewer);
    // Only allow a visualisation to reject a purge if receiving a
    // specific event trigger; Event::NONE indicates internal force through.
    if ( !vis->purge( this, e) && !EventGroup(e).is(Event::NONE))
        return false;

    vis->setVisible( this, false);

    _vlayers.erase(vis);
    if ( _xvis == vis)
        _xvis = nullptr;
    if ( _nMetricLayers > 0 && qobject_cast<MetricVisualiser*>(vis) != nullptr)
        _nMetricLayers--;

    return true;
}   // end purge


void FaceView::apply( BV* vis, const QPoint* mc)
{
    assert( vis);
    assert(_actor);
    assert(_viewer);

    const bool wasPresent = _vlayers.count(vis) > 0;
    assert( vis->isAvailable(_data));

    // Is the passed in visualisation exclusive?
    if ( _xvis != vis && (vis->isExclusive() || !vis->isToggled()))
    {
        if ( _xvis)
            _xvis->setVisible( this, false);
        _xvis = vis;
    }   // end if

    vis->apply( this, mc);
    vis->setVisible( this, true);

    _vlayers.insert(vis);

    if ( !wasPresent && qobject_cast<MetricVisualiser*>(vis) != nullptr)
        _nMetricLayers++;
}   // end apply


BV* FaceView::layer( const vtkProp* prop) const
{
    BV* vis = nullptr;
    if ( prop && _actor != prop)
    {
        for ( BV* v : _vlayers)    // Test all the visualisation layers
        {
            if ( v->belongs( prop, this))
            {
                vis = v;
                break;
            }   // end if
        }   // end for
    }   // end if
    return vis;
}   // end layer


void FaceView::syncActorDeltaToVisualisations()
{
    const cv::Matx44d& bmat = _data->model().transformMatrix(); // Data baseline transform
    const cv::Matx44d vmat = RVTK::toCV( _actor->GetMatrix());  // Actor's transform matrix
    const cv::Matx44d dmat = vmat * bmat.inv(); // Calc transform difference to be added

    // Collect all visualisation layers from all FaceViews (there is a more efficient
    // way to do this given that all visualisations are effectively singletons).
    VisualisationLayers vlayers;
    for ( FV* fv : _data->fvs())
    {
        const VisualisationLayers& vl = fv->visualisations();
        vlayers.insert( std::begin(vl), std::end(vl));
    }   // end for

    for ( BV* vis : vlayers)
        for ( FV* f : _data->fvs())
            vis->syncActorsToData( f, dmat);
}   // end syncActorDeltaToVisualisations


bool FaceView::isPointOnFace( const QPoint& p) const
{
    assert(_viewer);
    return _viewer && _actor ? _viewer->getPointedAt(p) == _actor : false;
}   // end isPointOnFace


bool FaceView::projectToSurface( const QPoint& p, cv::Vec3f& v) const
{
    assert(_viewer);
    return _viewer->calcSurfacePosition( _actor, p, v);
}   // end projectToSurface


double FaceView::opacity() const
{
    return _actor ? _actor->GetProperty()->GetOpacity() : 1.0;
}   // end opacity


void FaceView::setOpacity( double v)
{
    assert(_actor);
    _actor->GetProperty()->SetOpacity(v);
}   // end setOpacity


QColor FaceView::colour() const
{
    return _baseCol;
}   // end colour


void FaceView::setColour( const QColor& c)
{
    assert(_actor);
    _actor->GetProperty()->SetColor( c.redF(), c.greenF(), c.blueF());
    _baseCol = c;
}   // end setColour


void FaceView::setBackfaceCulling( bool v)
{
    assert(_actor);
    _actor->GetProperty()->SetBackfaceCulling( v);
}   // end setBackfaceCulling


bool FaceView::backfaceCulling() const
{
    return _actor ? _actor->GetProperty()->GetBackfaceCulling() : false;
}   // end backfaceCulling


void FaceView::setWireframe( bool v)
{
    assert(_actor);
    vtkProperty* p = _actor->GetProperty();
    if (v)
    {
        p->SetEdgeColor(0.0, 0.7, 0.1);
        //p->SetLineWidth(1.0f);
    }   // end if
    p->SetEdgeVisibility(v);
}   // end setWireframe


bool FaceView::wireframe() const
{
    return _actor ? _actor->GetProperty()->GetEdgeVisibility() : false;
}   // end wireframe


void FaceView::setTextured( bool v)
{
    assert(_actor);
    if ( v && _texture)
    {
        _actor->GetProperty()->SetColor( 1.0, 1.0, 1.0);    // Set the base colour to white
        _actor->SetTexture( _texture);
    }   // end if
    else
    {
        setColour(_baseCol);
        _actor->SetTexture( nullptr);
    }   // end else
    _updateModelLighting();
}   // end setTextured


bool FaceView::textured() const
{
    return _actor ? _actor->GetTexture() != nullptr : false;
}   // end textured


bool FaceView::canTexture() const
{
    assert(_actor);
    return _texture != nullptr;
}   // end canTexture


void FaceView::setActiveSurface( SurfaceMetricsMapper* s)
{
    if ( _smm)
    {
        _smm->remove(this);
        _smm = nullptr;
        setColour(_baseCol);
    }   // end if

    if ( !_actor)
        return;

    if ( s)
    {
        _smm = s;
        _smm->add(this);
        _actor->GetProperty()->SetColor( 1.0, 1.0, 1.0);    // Set the base colour to white
    }   // end if

    _updateModelLighting();
}   // end setActiveSurface


SurfaceMetricsMapper* FaceView::activeSurface() const { return _smm;}


void FaceView::_updateModelLighting()
{
    // Set the correct lighting
    vtkProperty* property = _actor->GetProperty();
    property->SetAmbient( textured() ? 1.0 : 0.0);
    property->SetDiffuse( 1.0);
    property->SetSpecular( 0.0);
}   // end _updateModelLighting
