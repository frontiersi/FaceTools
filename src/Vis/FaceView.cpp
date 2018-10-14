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

#include <FaceView.h>
#include <FaceModel.h>
#include <FaceModelViewer.h>
#include <SurfaceDataMapper.h>
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
using FaceTools::Vis::SurfaceDataMapper;
using FaceTools::FMV;
using FaceTools::FM;


// public
FaceView::FaceView( FM* fm, FMV* viewer)
    : _data(fm), _actor(nullptr), _texture(nullptr), _viewer(nullptr), _pviewer(nullptr),
      _sdmap(nullptr), _xvis(nullptr), _nMetricLayers(0)
{
    assert(viewer);
    assert(fm);
    _data->_fvs.insert(this);
    setViewer(viewer);
    reset();
}   // end ctor


FaceView::FaceView( const FaceView& fv)
{
    *this = fv;
}   // end ctor


FaceView& FaceView::operator=( const FaceView& fv)
{
    _data = fv.data();
    _data->_fvs.insert(this);
    _actor = nullptr;
    _texture = nullptr;
    _viewer = nullptr;
    return *this;
}   // end operator=


// public
FaceView::~FaceView()
{
    remove();   // Remove all
    setViewer(nullptr);
    _data->_fvs.erase(this);
    if ( _actor)
        _actor->Delete();
    _texture = nullptr;
}   // end dtor


// public
void FaceView::setViewer( FMV* viewer)
{
    if ( _viewer)
    {
        _viewer->detach(this);
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](BV* v){v->clear(this);});
        _viewer->remove(_actor);
    }   // end if

    _pviewer = _viewer;
    _viewer = viewer;

    if ( _viewer)
    {
        _viewer->add(_actor);
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](BV* v){v->apply(this);});
        _viewer->attach(this);
    }   // end if
}   // end setViewer


// public
void FaceView::reset()
{
    assert(_viewer);
    std::cerr << "[INFO] FaceTools::Vis::FaceView::reset: <" << this << "> on viewer <" << _viewer << ">" << std::endl;

    bool bface = false;
    bool tex = false;
    double op = 1.0;
    QColor cl(255,255,255);

    setActiveSurface(nullptr);

    if ( _actor)
    {
        bface = backfaceCulling();
        tex = textured();
        op = opacity();
        cl = colour();

        _viewer->remove(_actor);    // Remove the actor
        _actor->Delete();
        _actor = nullptr;
    }   // end if

    // Create the new actor from the data
    RVTK::VtkActorCreator ac;
    _polymap.clear();
    ac.setObjToVTKUniqueFaceMap( &_polymap);
    const RFeatures::ObjModel* model = _data->info()->cmodel();
    _actor = ac.generateActor( model, _texture);
    if ( !_texture)
        std::cerr << "[INFO] FaceTools::Vis::FaceView::reset: No texture found!" << std::endl;

    setBackfaceCulling(bface);
    setTextured(tex);
    setOpacity(op);
    setColour(cl);

    _viewer->add(_actor);   // Re-add the newly generated actor

    // Re-apply the old visualisation layers - now unavailable ones are left unapplied.
    auto vlayers = _vlayers;
    _vlayers.clear();
    _xvis = nullptr;
    _nMetricLayers = 0;
    std::for_each( std::begin(vlayers), std::end(vlayers), [this](BV* v){this->apply(v);});
}   // end reset


// public
void FaceView::remove( BV* vis)
{
    assert(_viewer);
    if ( !vis)
    {
        if ( !_vlayers.empty())
            remove( *_vlayers.begin());
        return;
    }   // end if
    vis->clear(this);
    _vlayers.erase(vis);
    if ( _xvis == vis)
        _xvis = nullptr;

    if ( _nMetricLayers > 0 && qobject_cast<MetricVisualiser*>(vis) != nullptr)
        _nMetricLayers--;
}   // end remove


// public
bool FaceView::apply( BV* vis, const QPoint* mc)
{
    assert( vis);
    assert(_actor);
    assert(_viewer);

    if ( _vlayers.count(vis) > 0)   // Already applied
        return true;

    if ( !vis->isAvailable(_data))
        return false;

    _data->lockForRead();
    vis->apply( this, mc);
    _vlayers.insert(vis);
    if ( !vis->isToggled() || vis->isExclusive())
    {
        assert(_xvis == nullptr);
        _xvis = vis;
    }   // end if
    _data->unlock();

    if ( qobject_cast<MetricVisualiser*>(vis) != nullptr)
        _nMetricLayers++;

    return isApplied(vis);
}   // end apply


// public
bool FaceView::isApplied( const BV *vis) const { return vis && _vlayers.count(const_cast<BV*>(vis)) > 0;}


// public
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


// public
bool FaceView::isPointOnFace( const QPoint& p) const
{
    assert(_viewer);
    return _viewer->getPointedAt(p) == _actor;
}   // end isPointOnFace


// public
bool FaceView::projectToSurface( const QPoint& p, cv::Vec3f& v) const
{
    assert(_viewer);
    return _viewer->calcSurfacePosition( _actor, p, v);
}   // end projectToSurface


// public
void FaceView::setPickable( bool v) { _actor->SetPickable(v);}
bool FaceView::pickable() const { return _actor->GetPickable();}


// public
double FaceView::opacity() const
{
    assert(_actor);
    return _actor->GetProperty()->GetOpacity();
}   // end opacity

void FaceView::setOpacity( double v)
{
    assert(_actor);
    _actor->GetProperty()->SetOpacity(v);
}   // end setOpacity


// public
QColor FaceView::colour() const
{
    assert(_actor);
    double* vc = _actor->GetProperty()->GetColor();
    return QColor::fromRgbF( vc[0], vc[1], vc[2]);
}   // end colour

void FaceView::setColour( const QColor& c)
{
    assert(_actor);
    _actor->GetProperty()->SetColor( c.redF(), c.greenF(), c.blueF());
}   // end setColour


void FaceView::setBackfaceCulling( bool v)
{
    assert(_actor);
    _actor->GetProperty()->SetBackfaceCulling( v);
}   // end setBackfaceCulling


bool FaceView::backfaceCulling() const
{
    assert(_actor);
    return _actor->GetProperty()->GetBackfaceCulling();
}   // end backfaceCulling


void FaceView::setWireframe( bool v)
{
    assert(_actor);
    vtkProperty* p = _actor->GetProperty();
    if (v)
    {
        p->SetEdgeColor(0,0.7,0);
        p->SetLineWidth(0.4f);
    }   // end if
    p->SetEdgeVisibility(v);
}   // end setWireframe


bool FaceView::wireframe() const
{
    assert(_actor);
    return _actor->GetProperty()->GetEdgeVisibility();
}   // end wireframe


void FaceView::setTextured( bool v)
{
    assert(_actor);
    _actor->SetTexture( v && _texture ? _texture : nullptr);
    // Set the correct lighting
    vtkProperty* property = _actor->GetProperty();
    property->SetAmbient( textured() ? 1.0 : 0.0);
    property->SetDiffuse( 1.0);
    property->SetSpecular(0.0);
}   // end setTextured


bool FaceView::textured() const
{
    assert(_actor);
    return _actor->GetTexture() != nullptr;
}   // end textured


bool FaceView::canTexture() const
{
    assert(_actor);
    return _texture != nullptr;
}   // end canTexture


void FaceView::setActiveSurface( SurfaceDataMapper* s)
{
    if ( _sdmap)
    {
        _sdmap->remove(this);
        _sdmap = nullptr;
    }   // end if

    if ( !_actor)
        return;

    if ( s)
    {
        _sdmap = s;
        _sdmap->add(this);
    }   // end if
}   // end setActiveSurface


SurfaceDataMapper* FaceView::activeSurface() const { return _sdmap;}
