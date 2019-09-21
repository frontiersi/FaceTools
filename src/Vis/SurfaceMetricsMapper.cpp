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

#include <Vis/SurfaceMetricsMapper.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
#include <VtkTools.h>   // RVTK
#include <vtkCellData.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::SurfaceMetricsMapper;
using FaceTools::Vis::FV;
using FaceTools::FM;


SurfaceMetricsMapper::SurfaceMetricsMapper( const std::string& lb, bool mp, size_t d, float minv, float maxv)
    : _smapper( RVTK::SurfaceMapper::create( lb, [this](int id, size_t k){ return this->metric(id,k);}, mp, d)),
      _minr(minv), _maxr(maxv), _minv(minv), _maxv(maxv)
{
    _cmapper.setRangeLimits( _minr, _maxr);
    setVisibleRange( minv, maxv);
    setMinColour( QColor(0  ,   0, 255));   // Blue
    setMaxColour( QColor(255,   0,   0));   // Red
    setNumColours( 99);
    _cmapper.rebuild();
}   // end ctor


SurfaceMetricsMapper::~SurfaceMetricsMapper() {}


bool SurfaceMetricsMapper::isScalarMapping() const { return _smapper->ndimensions() == 1;}


void SurfaceMetricsMapper::setVisibleRange( float vmin, float vmax)
{
    _cmapper.setVisibleRange(vmin,vmax);
    _minv = _cmapper.minVisible();
    _maxv = _cmapper.maxVisible();
}   // end setVisibleRange


void SurfaceMetricsMapper::setNumColours( size_t v) { _cmapper.setNumColours( v);}
void SurfaceMetricsMapper::setMinColour( const QColor& c) { _cmapper.setMinColour(c);}
void SurfaceMetricsMapper::setMaxColour( const QColor& c) { _cmapper.setMaxColour(c);}

size_t SurfaceMetricsMapper::numColours() const { return _cmapper.numColours();}
QColor SurfaceMetricsMapper::minColour() const { return _cmapper.minColour();}
QColor SurfaceMetricsMapper::maxColour() const { return _cmapper.maxColour();}
vtkLookupTable* SurfaceMetricsMapper::scalarLookupTable() { return _cmapper.lookupTable().vtk();}


void SurfaceMetricsMapper::rebuild()
{
    _cmapper.rebuild();
    std::for_each( std::begin(_fvs), std::end(_fvs), [this](FV *fv){ this->refreshActorSurface(fv->actor());});
    _fvs.updateRenderers();
}   // end rebuild


bool SurfaceMetricsMapper::mapMetrics( FV *fv)
{
    const FM* fm = fv->data();
    if ( !init(fm))
        return false;

    if ( !_smapper->mapsPolys())
    {
        std::cerr << "[ERROR] FaceTools::Vis::SurfaceMetricsMapper::mapMetrics: Non-polygon mappings currently not supported!" << std::endl;
        return false;
    }   // end if

    _smapper->mapMetrics( fm->model(), fv->actor());
    done( fm);
    return true;
}   // end mapMetrics


void SurfaceMetricsMapper::remove( FV *fv)
{
    _fvs.erase(fv);
    vtkActor *actor = fv->actor();
    vtkCellData *celldata = RVTK::getPolyData(actor)->GetCellData();
    celldata->SetActiveScalars( "");
    celldata->SetActiveVectors( "");
    actor->GetMapper()->SetScalarVisibility( false);
}   // end removeActor


void SurfaceMetricsMapper::add( FV *fv)
{
    _fvs.insert(fv);
    vtkActor *actor = fv->actor();
    vtkCellData *celldata = RVTK::getPolyData(actor)->GetCellData();
    if ( isScalarMapping())
    {
        actor->GetMapper()->SetLookupTable( _cmapper.lookupTable().vtk());
        celldata->SetActiveScalars( label().c_str());
        actor->GetMapper()->SetScalarVisibility( true);
    }   // end if
    else
        celldata->SetActiveVectors( label().c_str());
    refreshActorSurface(actor);
}   // end add


// private
void SurfaceMetricsMapper::refreshActorSurface( vtkActor *actor)
{
    if ( isScalarMapping())
        actor->GetMapper()->SetScalarRange( double(minVisible()), double(maxVisible()));
}   // end refreshActorSurface
