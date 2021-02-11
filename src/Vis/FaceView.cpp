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

#include <Vis/FaceView.h>
#include <Vis/BaseVisualisation.h>
#include <Vis/MetricVisualiser.h>
#include <Vis/ColourVisualisation.h>
#include <ModelSelect.h>
#include <FaceModelCurvatureStore.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkPointData.h>
#include <vtkProperty.h>
#include <r3dvis/VtkTools.h>
#include <QColor>
#include <iostream>
#include <cassert>
using FaceTools::Vis::FaceView;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Vec3f;
using BV = FaceTools::Vis::BaseVisualisation;
using CV = FaceTools::Vis::ColourVisualisation;
using MS = FaceTools::ModelSelect;


// static definitions
bool FaceView::s_smoothLighting(false);
bool FaceView::s_interpolateShading(false);
const QColor FaceView::BASECOL(202, 188, 232);

void FaceView::setSmoothLighting( bool v)
{
    s_smoothLighting = v;
    MS::updateAllViews( []( FV* fv){ fv->reset();}, true);
}   // end setSmoothLighting


bool FaceView::smoothLighting() { return s_smoothLighting;}


void FaceView::setInterpolatedShading( bool v)
{
    s_interpolateShading = v;
    MS::updateAllViews( []( FV* fv){ fv->reset();}, true);
}   // end setInterpolatedShading


bool FaceView::interpolatedShading() { return s_interpolateShading;}


FaceView::FaceView( FM* fm, FMV* viewer)
    : _data(fm), _actor(nullptr), _texture(nullptr), _nrms(nullptr), _viewer(nullptr), _pviewer(nullptr),
      _cv(nullptr), _baseCol(FaceView::BASECOL), _minAllowedOpacity(0.1f), _maxAllowedOpacity(1.0f)
{
    assert(viewer);
    assert(fm);
    _data->addView(this);
    setViewer(viewer);
    reset();
}   // end ctor


void FaceView::resetNormals()
{
    const auto &rptr = FaceModelCurvatureStore::rvals( *_data);
    if ( rptr)
    {
        _nrms = rptr->normals();
        r3dvis::getPolyData( _actor)->GetPointData()->SetNormals( _nrms);
    }   // end if
}   // end resetNormals


void FaceView::copyFrom( const FaceView *fv)
{
    assert( fv);
    setBackfaceCulling( fv->backfaceCulling());
    setOpacity( fv->opacity());
    setColour( fv->colour());

    // Only apply visualisations if visible on the source view and available on this one.
    for ( BaseVisualisation* vl : fv->visualisations())
        if ( vl->isVisible(fv) && vl->isAvailable(this))
            apply(vl);

    resetNormals();
    _updateSurfaceProperties();
}   // end copyFrom


FaceView::~FaceView()
{
    while ( !_vlayers.empty())
        purge( *_vlayers.begin());
    setViewer(nullptr);
    _data->eraseView(this);
}   // end dtor


void FaceView::setViewer( FMV* nviewer)
{
    assert( nviewer != _viewer);

    VisualisationLayers onSet, xtraOnSet;
    if ( _viewer)
    {
        // Get the set of visualisations that are currently on
        for ( BV* vis : _vlayers)
            if ( vis->isVisible(this))
                onSet.insert(vis);
        // Turn them all off on this viewer (will try to turn them back on again after changing viewer).
        // Note that turning some visualisations off may turn others on so we'll need to add these too.
        for ( BV* vis : onSet)
            vis->setVisible( this, false);
        for ( BV* vis : _vlayers)   // Get any newly turned on ones too
            if ( vis->isVisible(this))
                xtraOnSet.insert(vis);
        // Purge all visualisations before reapplying the visible
        // ones that are also available on the new viewer.
        while ( !_vlayers.empty())
            purge( *_vlayers.begin());
        _viewer->remove(_actor);
        _viewer->detach(this);
    }   // end if

    _pviewer = _viewer;
    _viewer = nviewer;

    if ( _viewer)
    {
        _viewer->add(_actor);
        _viewer->attach(this);
        // The default set of turned on visualisations will be the visualisations
        // resulting from turning *off* the original set. If we can turn on ones in
        // the original (onSet) set that caused these *off* visualisations to be on,
        // they should be replaced.
        for ( BV* vis : xtraOnSet)
            if ( vis->isAvailable(this))
                apply( vis);
        for ( BV* vis : onSet)
            if ( vis->isAvailable(this))
                apply( vis);
    }   // end if
}   // end setViewer


void FaceView::reset()
{
    assert(_viewer);

    // Collect the old visible layers to reapply afterwards
    VisualisationLayers oldVisLayers;
    for ( BV* vis : _vlayers)
        if ( vis->isVisible(this))
            oldVisLayers.insert( vis);

    const bool tex = textured();
    const bool wframe = wireframe();
    const bool bface = backfaceCulling();
    const float op = opacity();
    const QColor cl = colour();

    if ( _actor)
    {
        _viewer->remove(_actor);    // Remove the actor
        _actor = nullptr;
        _texture = nullptr;
    }   // end if

    // Create the new actor from the data
    _actor = r3dvis::VtkActorCreator::generateActor( _data->mesh());
    _texture = _actor->GetTexture();

    resetNormals();

    setBackfaceCulling(bface);
    setWireframe(wframe);
    setTextured(tex);
    setOpacity(op);
    setColour(cl);

    _viewer->add(_actor);   // Re-add the newly generated actor

    // Re-apply the visualisation layers after purging existing
    while ( !_vlayers.empty())
        purge( *_vlayers.begin());

    for ( BV* vis : oldVisLayers)
        apply(vis);
}   // end reset


void FaceView::purge( BV* vis)
{
    if ( _vlayers.count(vis) == 0)
        return;
    vis->setVisible( this, false);
    vis->purge( this);
    _vlayers.erase(vis);
}   // end purge


void FaceView::apply( BV* vis)
{
    assert( vis);
    assert(_actor);
    assert(_viewer);
    assert( vis->isAvailable(this));
    _vlayers.insert(vis);
    vis->refresh( this);
    vis->syncTransform( this);
    vis->setVisible( this, true);
}   // end apply


bool FaceView::isApplied( const BV *vis) const { return _vlayers.count(const_cast<BV*>(vis)) > 0;}


const BV* FaceView::layer( const vtkProp* prop) const { return _layer(prop);}
BV* FaceView::layer( const vtkProp *prop) { return _layer(prop);}


BV* FaceView::_layer( const vtkProp* prop) const
{
    if ( prop && _actor != prop)
        for ( BV *vis : _vlayers)
            if ( vis->belongs( prop, this) && vis->isVisible( this))
                return vis;
    return nullptr;
}   // end _layer


void FaceView::pokeTransform( const vtkMatrix4x4 *t)
{
    _actor->PokeMatrix( const_cast<vtkMatrix4x4*>(t));
    for ( BV* vis : _vlayers)
        vis->syncTransform( this);
}   // end pokeTransform


bool FaceView::isPointOnFace( const QPoint& p) const
{
    assert(_viewer);
    return p.x() >= 0 ? _viewer->getPointedAt(p) == _actor : false;
}   // end isPointOnFace


bool FaceView::projectToSurface( const QPoint& p, Vec3f& v) const
{
    assert(_viewer);
    return _viewer->calcSurfacePosition( _actor, p, v);
}   // end projectToSurface


bool FaceView::overlaps() const
{
    assert(_viewer);
    const r3d::Bounds& ibds = *data()->bounds()[0];
    const FVS& fvs = _viewer->attached();
    for ( const FV *tv : fvs)
    {
        if ( tv != this)
        {
            const FM *tfm = tv->data();
            if ( ibds.intersects( *tfm->bounds()[0]))
                return true;
        }   // end if
    }   // end for
    return false;
}   // end overlaps


float FaceView::minAllowedOpacity() const
{
    // Also check availability of visualisation since it may not have been removed yet
    float opc = _minAllowedOpacity;
    for ( const BV* vis : _vlayers)
        if ( vis->isVisible(this) && vis->isAvailable(this))
            opc = std::max<float>( opc, vis->minAllowedOpacity());
    return opc;
}   // end minAllowedOpacity


float FaceView::maxAllowedOpacity() const
{
    // Also check availability of visualisation since it may not have been removed yet
    float opc = _maxAllowedOpacity;
    for ( const BV* vis : _vlayers)
        if ( vis->isVisible(this) && vis->isAvailable(this))
            opc = std::min<float>( opc, vis->maxAllowedOpacity());
    return opc;
}   // end maxAllowedOpacity


void FaceView::setMaxAllowedOpacity( float v)
{
    _maxAllowedOpacity = std::min( 1.0f, std::max( 0.0f, v));
    _minAllowedOpacity = std::min( _minAllowedOpacity, _maxAllowedOpacity);
}   // end setMaxAllowedOpacity


void FaceView::setMinAllowedOpacity( float v)
{
    _minAllowedOpacity = std::min( 1.0f, std::max( 0.0f, v));
    _maxAllowedOpacity = std::max( _minAllowedOpacity, _maxAllowedOpacity);
}   // end setMinAllowedOpacity


void FaceView::setOpacity( float v)
{
    assert(_actor);
    _actor->GetProperty()->SetOpacity( std::max( minAllowedOpacity(), std::min( v, maxAllowedOpacity())));
}   // end setOpacity


float FaceView::opacity() const { return _actor ? _actor->GetProperty()->GetOpacity() : 1.0f;}


QColor FaceView::colour() const { return _baseCol;}


void FaceView::setColour( const QColor& c)
{
    assert(_actor);
    _baseCol = c;
    _updateSurfaceProperties();
}   // end setColour


void FaceView::setBackfaceCulling( bool v)
{
    assert(_actor);
    _actor->GetProperty()->SetBackfaceCulling( v);
}   // end setBackfaceCulling


bool FaceView::backfaceCulling() const
{
    return _actor ? _actor->GetProperty()->GetBackfaceCulling() : true;
}   // end backfaceCulling


void FaceView::setWireframe( bool v)
{
    assert(_actor);
    vtkProperty* p = _actor->GetProperty();
    if (v)
        p->SetEdgeColor(0.0, 0.7, 0.1);
    p->SetEdgeVisibility(v);
}   // end setWireframe


bool FaceView::wireframe() const
{
    return _actor && _actor->GetProperty()->GetEdgeVisibility();
}   // end wireframe



bool FaceView::textured() const { return _actor && _actor->GetTexture() != nullptr;}

bool FaceView::canTexture() const { return _texture != nullptr;}

void FaceView::setTextured( bool v)
{
    if ( canTexture() && v)
        _actor->SetTexture( _texture);
    else
        _actor->SetTexture( nullptr);
    // Turning the texture on or off will always turn off the active cell/point
    // surface mapping because we are toggling the texture with the plain surface.
    _updateSurfaceProperties();
}   // end setTextured


void FaceView::setActiveColours( CV* cv)
{
    _actor->GetMapper()->SetLookupTable(nullptr);

    if ( cv != _cv)
    {
        if ( _cv)
            _cv->deactivate(this);
        _cv = cv;
        if ( cv)
            cv->activate(this);
    }   // end if

    if ( _cv)
    {
        vtkLookupTable *lut = const_cast<vtkLookupTable*>(_cv->lookupTable());
        _actor->GetMapper()->SetLookupTable( lut);
        refreshColourMap();
    }   // end if
    _updateSurfaceProperties();
}   // end setActiveColours


void FaceView::refreshColourMap()
{
    if ( _cv)
    {
        _cv->refreshColourMap(this);
        _actor->GetMapper()->SetScalarRange( _cv->minVisible(), _cv->maxVisible());
    }   // end if
}   // end refreshColourMap


void FaceView::addCellsArray( vtkFloatArray *arr) { CV::addCellsArray(_actor, arr);}
void FaceView::addPointsArray( vtkFloatArray *arr) { CV::addPointsArray(_actor, arr);}
void FaceView::setActiveCellScalars( const char *n) { CV::setActiveCellScalars(_actor, n);}
void FaceView::setActiveCellVectors( const char *n) { CV::setActiveCellVectors(_actor, n);}
void FaceView::setActivePointScalars( const char *n) { CV::setActivePointScalars(_actor, n);}
void FaceView::setActivePointVectors( const char *n) { CV::setActivePointScalars(_actor, n);}


void FaceView::_updateSurfaceProperties()
{
    vtkProperty* property = _actor->GetProperty();
    property->SetInterpolationToFlat();
    if ( s_smoothLighting)
        property->SetInterpolationToPhong();

    property->SetRoughness( 1.0);
    property->SetEmissiveFactor( 0.0, 0.0, 0.0);
    property->SetShading( false);
    property->SetMetallic( 0.0);

    property->SetColor( 1, 1, 1);
    if ( !textured() && !_cv)
        property->SetColor( _baseCol.redF(), _baseCol.greenF(), _baseCol.blueF());

    property->SetAmbient( textured() ? 1.0 : 0.0);
    property->SetDiffuse( textured() ? 0.0 : 1.0);
    property->SetSpecular( 0.0);

    // Interpolation of scalar mapped data?
    _actor->GetMapper()->SetInterpolateScalarsBeforeMapping(false);
    if ( s_interpolateShading)
        _actor->GetMapper()->SetInterpolateScalarsBeforeMapping(true);

    _actor->GetMapper()->SetScalarVisibility( _cv != nullptr);  // Not actually necessary
}   // end _updateSurfaceProperties
