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

#include <ScalarVisualisation.h>
#include <FaceModelSurfaceData.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkDataSetAttributes.h>
#include <vtkCellData.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
#include <PolySurfaceCurvScalarMapper.h>
using RFeatures::ObjModelCurvatureMetrics;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::Vis::ScalarMappingFn;
using FaceTools::FaceModelSurfaceData;
using FaceTools::SurfaceData;
using FaceTools::FVS;
using FaceTools::FM;


namespace {
class CurvMapper : public RVTK::PolySurfaceCurvScalarMapper
{
public:
    CurvMapper( const ScalarMappingFn& func, const ObjModelCurvatureMetrics* cm,
                vtkActor* actor, const IntIntMap* lookup, const std::string& mname)
        : RVTK::PolySurfaceCurvScalarMapper( cm, actor, lookup, mname), _func(func) {}
private:
    ScalarMappingFn _func;
    float getCurvMetric( int faceIdx) const override { return _func( _cmetrics, faceIdx);}
};  // end class
}   // end namespace


ScalarVisualisation::ScalarVisualisation( const QString& d, const ScalarMappingFn& mf, float minv, float maxv, const QIcon& icon)
    : BaseVisualisation(d, icon),
      _scmap( d.toStdString()), _mfunc(mf)
{
    _scmap.setRangeLimits( minv, maxv);
    _scmap.setVisibleLimits( minv, maxv);
    _scmap.rebuild();
}   // end ctor


// public
bool ScalarVisualisation::isAvailable( const FM* fm) const
{
    return FaceModelSurfaceData::get()->isAvailable(fm);
}   // end isAvailable


// public
void ScalarVisualisation::apply( FV* fv, const QPoint*)
{
    FM* fm = fv->data();
    // If no views have yet been mapped to FV's underlying data, may need to
    // adjust the visible range across all views with this scalar mapping applied.
    //const bool doremap = _mappings.count(fm) == 0;

    std::pair<float,float> minmax;
    if ( !_mapped.has(fv))
    {
        SurfaceData::RPtr msd = FaceModelSurfaceData::rdata(fm); // Scoped read lock
        CurvMapper mapper( _mfunc, msd->metrics, fv->actor(), &fv->polyLookups(), getDisplayName().toStdString());
        mapper.mapActor();
        _mapped.insert(fv);

        // Okay to overwrite the mapping min,max for the same model.
        mapper.getMappedRange( &minmax.first, &minmax.second);
        _mappings[fm] = minmax;
    }   // end if

    /*
    if ( doremap)
        remapColourRangeAcrossModels();
    */

    fv->setActiveScalars( &_scmap);
}   // end apply


// public
void ScalarVisualisation::remove( FV* fv)
{
    if ( fv->activeScalars() == &_scmap)
        fv->setActiveScalars(nullptr);
    _mapped.erase(fv);
}   // end remove


// protected
void ScalarVisualisation::purge( FV* fv)
{
    const size_t psize = _mapped.size();
    remove(fv);
    if ( _mapped.size() < psize)
    {
        // If none of the underlying data's views are mapped, the mapping range may need
        // to be readjusted to possible narrowing of the range across the remaining views.
        FM* fm = fv->data();
        const FVS& fvs = fm->fvs();
        if ( !std::any_of( std::begin(fvs), std::end(fvs), [this](FV* fv){ return _mapped.has(fv);}))
        {
            _mappings.erase(fm);
            //remapColourRangeAcrossModels();
        }   // end if
    }   // end if
}   // end purge


// private
void ScalarVisualisation::remapColourRangeAcrossModels()
{
    float nmin = FLT_MAX;
    float nmax = -FLT_MAX;
    for ( const auto& p : _mappings)
    {
        nmin = std::min( nmin, p.second.first);
        nmax = std::max( nmax, p.second.second);
    }   // end for

    if ( !_mappings.empty() && (nmin != _scmap.minRange() || nmax != _scmap.maxRange()))
    {
        _scmap.setRangeLimits( nmin, nmax);
        _scmap.rebuild();
    }   // end if
}   // end remapColourRangeAcrossModels
