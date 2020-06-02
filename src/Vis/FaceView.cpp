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

#include <Vis/FaceView.h>
#include <Vis/BaseVisualisation.h>
#include <Vis/MetricVisualiser.h>
#include <Vis/ScalarVisualisation.h>
#include <Vis/VectorVisualisation.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <FaceModelCurvature.h>
#include <FaceModel.h>
#include <vtkProperty.h>
#include <r3dvis/VtkTools.h>
#include <QColor>
#include <iostream>
#include <cassert>
using FaceTools::Vis::FaceView;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::VectorVisualisation;
using FaceTools::FMV;
using FaceTools::FM;
using FaceTools::Vec3f;
using BV = FaceTools::Vis::BaseVisualisation;
using MS = FaceTools::Action::ModelSelector;


// static definitions
bool FaceView::s_smoothLighting(false);
bool FaceView::s_interpolateShading(false);

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
      _smm(nullptr), _vmm(nullptr), _baseCol(202,188,232), _xvis(nullptr)
{
    assert(viewer);
    assert(fm);
    _data->addView(this);
    setViewer(viewer);
    reset();
}   // end ctor


void FaceView::resetNormals()
{
    FaceModelCurvature::RPtr cmap = FaceModelCurvature::rmetrics( _data);  // May be null if not yet processed curvature
    if ( cmap)
    {
        vtkSmartPointer<vtkFloatArray> nrms = r3dvis::makeNormals( *cmap);
        nrms->SetName("Normals");
        assert(nrms);
        vtkPolyData *pd = r3dvis::getPolyData( _actor);
        pd->GetPointData()->SetNormals( nrms);
        _nrms = nrms;
    }   // end if
}   // end resetNormals


void FaceView::copyFrom( const FaceView* fv)
{
    assert( _actor != fv->_actor);
    assert( _viewer != fv->_viewer);

    setBackfaceCulling( fv->backfaceCulling());
    setOpacity( fv->opacity());
    setColour( fv->colour());

    for ( BaseVisualisation* vl : fv->visualisations())
        if ( vl->isVisible(fv)) // Only apply on this view if visible on the source view
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

    VisualisationLayers visLayers;
    if ( _viewer)
    {
        for ( BV* vis : _vlayers)
        {
            if ( vis->isVisible(this))
            {
                visLayers.insert( vis);
                vis->setVisible( this, false);
            }   // end if
        }   // end for

        _viewer->remove(_actor);
        _viewer->detach(this);
    }   // end if

    _pviewer = _viewer;
    _viewer = nviewer;

    if ( _viewer)
    {
        _viewer->add(_actor);
        _viewer->attach(this);
        for ( BV* vis : visLayers)
            _setVisible( vis, true);
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


#ifndef NDEBUG
namespace {

std::string viewerCode( const FMV *fmv)
{
    int vnum = -1;
    const int N = int(MS::viewers().size());
    for ( int i = 0; i < N; ++i)
    {
        if ( MS::viewers()[i] == fmv)
        {
            vnum = i;
            break;
        }   // end if
    }   // end for

    std::string vcode;
    switch (vnum)
    {
        case 0:
            vcode = "LEFT";
            break;
        case 1:
            vcode = "MAIN";
            break;
        case 2:
            vcode = "RGHT";
            break;
        default:
            vcode = "ERROR!";
    }   // end switch

    return vcode;
}   // end viewerCode

}   // end namespace
#endif


void FaceView::purge( BV* vis)
{
    if ( _vlayers.count(vis) == 0)
        return;

#ifndef NDEBUG
    std::cerr << " -- [" << viewerCode(_viewer) << "] : " << vis->name() << std::endl;
#endif
    assert(vis);
    assert(_viewer);
    vis->setVisible( this, false);
    vis->purge( this);
    _vlayers.erase(vis);
    if ( _xvis == vis)
        _xvis = nullptr;
}   // end purge


void FaceView::apply( BV* vis, const QPoint* mc)
{
    assert( vis);
    assert(_actor);
    assert(_viewer);
    assert( vis->isAvailable(this, mc));

    // Is the passed in visualisation exclusive?
    if ( !vis->isToggled())
    {
        if ( _xvis && _xvis != vis)
            _xvis->setVisible( this, false);
        _xvis = vis;
    }   // end if

    if ( _vlayers.count(vis) == 0)
    {
        _vlayers.insert(vis);
#ifndef NDEBUG
        std::cerr << " ++ [" << viewerCode(_viewer) << "] : " << vis->name() << std::endl;
#endif
        vis->apply( this, mc);
    }   // end if

    _setVisible( vis, true);
}   // end apply


void FaceView::_setVisible( BV *vis, bool v)
{
    vis->setVisible( this, v);
    vis->refreshState( this);
    vis->setVisible( this, v);
    vis->syncWithViewTransform( this);
}   // end _setVisible


bool FaceView::isApplied( const BV *vis) const { return _vlayers.count(const_cast<BV*>(vis)) > 0;}


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


void FaceView::pokeTransform( const vtkMatrix4x4 *t)
{
    _actor->PokeMatrix( const_cast<vtkMatrix4x4*>(t));
    for ( BV* vis : _vlayers)
        vis->syncWithViewTransform( this);
}   // end pokeTransform


bool FaceView::isPointOnFace( const QPoint& p) const
{
    assert(_viewer);
    return _viewer && _actor && p.x() >= 0 ? _viewer->getPointedAt(p) == _actor : false;
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
    float opc = 0.1f;   // Absolute minimum level of opacity (prevent complete invisibility)
    for ( const BV* vis : _vlayers)
        if ( vis->isVisible(this))
            opc = std::max<float>( opc, vis->minAllowedOpacity());
    return opc;
}   // end minAllowedOpacity


float FaceView::maxAllowedOpacity() const
{
    float opc = 1.0f;
    for ( const BV* vis : _vlayers)
        if ( vis->isVisible(this))
            opc = std::min<float>( opc, vis->maxAllowedOpacity());
    return opc;
}   // end maxAllowedOpacity


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


void FaceView::setTextured( bool v)
{
    assert(_actor);
    vtkProperty* property = _actor->GetProperty();

    if ( v && _texture)
    {
        property->SetColor( 1.0, 1.0, 1.0);    // Set the base colour to white
        _actor->SetTexture( _texture);
    }   // end if
    else
    {
        setColour(_baseCol);
        _actor->SetTexture( nullptr);
    }   // end else

    setActiveScalars( nullptr);
}   // end setTextured


bool FaceView::textured() const { return _actor && _actor->GetTexture() != nullptr;}


bool FaceView::canTexture() const { return _texture != nullptr;}


void FaceView::setActiveScalars( ScalarVisualisation* s)
{
    if ( _smm && _smm != s)
    {
        _smm->deactivate(this);
        _smm = nullptr;
    }   // end if

    if ( s && _smm != s)
    {
        _smm = s;
        _smm->activate(this);
    }   // end if

    if ( _smm)
    {
        _actor->GetProperty()->SetColor( 1.0, 1.0, 1.0);    // Set the base colour to white
        _actor->SetTexture( nullptr);
    }   // end if
    else
        setColour(_baseCol);

    setActiveVectors( _vmm);
}   // end setActiveScalars


ScalarVisualisation* FaceView::activeScalars() const { return _smm;}


void FaceView::setActiveVectors( VectorVisualisation* v)
{
    if ( _vmm && _vmm != v)
    {
        _vmm->deactivate(this);
        _vmm = nullptr;
    }   // end if

    if ( v && _vmm != v)
    {
        _vmm = v;
        _vmm->activate(this);
    }   // end if

    if ( _vmm)
        _vmm->setScalarMapping( this, _smm);

    _updateSurfaceProperties();
}   // end setActiveVectors


VectorVisualisation* FaceView::activeVectors() const { return _vmm;}


void FaceView::_updateSurfaceProperties()
{
    vtkProperty* property = _actor->GetProperty();
    property->SetAmbient( textured() ? 1.0 : 0.0);
    property->SetDiffuse( 1.0);
    property->SetSpecular( 0.0);

    // Lighting interpolation?
    property->SetInterpolationToFlat();
    if ( s_smoothLighting)
        property->SetInterpolationToPhong();

    // Interpolation of scalar mapped data?
    _actor->GetMapper()->SetInterpolateScalarsBeforeMapping(false);
    if ( s_interpolateShading)
        _actor->GetMapper()->SetInterpolateScalarsBeforeMapping(true);
}   // end _updateSurfaceProperties
