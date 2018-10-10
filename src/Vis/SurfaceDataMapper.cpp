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

#include <SurfaceDataMapper.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <VtkTools.h>   // RVTK
#include <algorithm>
#include <cassert>
using FaceTools::Vis::SurfaceDataMapper;
using FaceTools::Vis::FV;
using FaceTools::FM;


SurfaceDataMapper::SurfaceDataMapper( const std::string& lb, bool mp, size_t d, float minv, float maxv)
    : _smapper( RVTK::SurfaceMapper::create( lb, [this](int id, size_t k){ return this->metric(id,k);}, mp, d)),
      _minr(minv), _maxr(maxv), _minv(minv), _maxv(maxv)
{
    _cmapper.setRangeLimits( _minr, _maxr);
    setVisibleLimits( minv, maxv);
    setMinColour( QColor(0  ,   0, 255));   // Blue
    setMaxColour( QColor(255,   0,   0));   // Red
    setNumColours( 20);
    _cmapper.rebuild();
}   // end ctor


bool SurfaceDataMapper::isScalarMapping() const { return _smapper->ndimensions() == 1;}


void SurfaceDataMapper::setVisibleLimits( float vmin, float vmax)
{
    _cmapper.setVisibleLimits(vmin,vmax);
    _minv = _cmapper.minVisible();
    _maxv = _cmapper.maxVisible();
}   // end setVisibleLimits


void SurfaceDataMapper::setNumColours( size_t v) { _cmapper.setNumColours( v);}
void SurfaceDataMapper::setMinColour( const QColor& c) { _cmapper.setMinColour(c);}
void SurfaceDataMapper::setMaxColour( const QColor& c) { _cmapper.setMaxColour(c);}

size_t SurfaceDataMapper::numColours() const { return _cmapper.numColours();}
QColor SurfaceDataMapper::minColour() const { return _cmapper.minColour();}
QColor SurfaceDataMapper::maxColour() const { return _cmapper.maxColour();}
vtkLookupTable* SurfaceDataMapper::scalarLookupTable() { return _cmapper.lookupTable().vtk();}


void SurfaceDataMapper::rebuild()
{
    _cmapper.rebuild();
    std::for_each( std::begin(_fvs), std::end(_fvs), [this](FV *fv){ this->refreshActorSurface(fv->actor());});
    _fvs.updateRenderers();
}   // end rebuild


bool SurfaceDataMapper::mapMetrics( FV *fv)
{
    const FM* fm = fv->data();
    if ( !init(fm))
        return false;

    const IntIntMap *lmap = nullptr;
    if ( _smapper->mapsPolys())   // Currently, only polygon lookups handled
        lmap = &fv->polyLookups();

    assert( lmap);
    if ( !lmap)
    {
        std::cerr << "[ERROR] FaceTools::Vis::SurfaceDataMapper::mapMetrics: Non-polygon mappings currently not supported!" << std::endl;
        return false;
    }   // end if

    _smapper->mapMetrics( fm->info()->cmodel(), lmap, fv->actor());
    return true;
}   // end mapMetrics


void SurfaceDataMapper::remove( FV *fv)
{
    if ( _fvs.has(fv))
    {
        _fvs.erase(fv);
        vtkActor *actor = fv->actor();
        vtkCellData *celldata = RVTK::getPolyData(actor)->GetCellData();
        celldata->SetActiveScalars( "");
        celldata->SetActiveVectors( "");
        actor->GetMapper()->SetScalarVisibility( false);
    }   // end if
}   // end removeActor


void SurfaceDataMapper::add( FV *fv)
{
    if ( !_fvs.has(fv))
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
    }   // end if
}   // end add


// private
void SurfaceDataMapper::refreshActorSurface( vtkActor *actor)
{
    if ( isScalarMapping())
        actor->GetMapper()->SetScalarRange( minVisible(), maxVisible());
}   // end refreshActorSurface
