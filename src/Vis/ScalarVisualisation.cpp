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
using QTools::ColourMappingWidget;


ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i, const QKeySequence& k)
    : BaseVisualisation(d,i,k), _scmap( d.toStdString()), _mfunc(nullptr)
{
}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i)
    : BaseVisualisation(d,i), _scmap( d.toStdString()), _mfunc(nullptr)
{
}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d)
    : BaseVisualisation(d), _scmap( d.toStdString()), _mfunc(nullptr)
{
}   // end ctor


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


// private
std::pair<float,float> ScalarVisualisation::mapActor( FV* fv) const
{
    SurfaceData::RPtr msd = FaceModelSurfaceData::rdata(fv->data()); // Scoped read lock
    assert( _mfunc != nullptr);
    CurvMapper mapper( _mfunc, msd->metrics, fv->actor(), &fv->polyLookups(), getDisplayName().toStdString());
    mapper.mapActor();
    float minval, maxval;
    mapper.getMappedRange( &minval, &maxval);
    return std::pair<float,float>( minval, maxval);
}   // end mapActor


// public
bool ScalarVisualisation::isAvailable( const FM* fm) const
{
    return FaceModelSurfaceData::get()->isAvailable(fm) && _mfunc != nullptr;
}   // end isAvailable


// public
void ScalarVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _mappings.count(fv) == 0)
        _mappings[fv] = mapActor(fv);

    // If the mapped min,max for this visualisation widens the scalar range,
    // remap the surfaces of the currently mapped views by rebuilding the lookup table.
    const std::pair<float,float>& mm = _mappings.at(fv);
    const std::pair<float,float>& rl = _scmap.rangeLimits();
    if ( mm.first < rl.first || mm.second > rl.second)
    {
        const float nmin = std::min( mm.first, rl.first);
        const float nmax = std::max( mm.second, rl.second);
        _scmap.setRangeLimits( nmin, nmax);
        _scmap.rebuild();
    }   // end if

    fv->setActiveScalars( &_scmap);
}   // end apply


// public
void ScalarVisualisation::remove( FV* fv)
{
    if ( _mappings.count(fv) == 0)
        return;

    std::pair<float,float> mm = _mappings.at(fv);
    const std::pair<float,float>& rl = _scmap.rangeLimits();
    _mappings.erase(fv);
    fv->setActiveScalars(nullptr);
    
    // If removing the mapping for this FaceView narrows the scalar range, find the new
    // range by iterating over the already mapped views, and allow the views to remap themselves.
    if ( mm.first <= rl.first || mm.second >= rl.second)
    {
        for ( const auto& p : _mappings)
        {
            mm.first = std::min( mm.first, p.second.first);
            mm.second = std::max( mm.second, p.second.second);
        }   // end for
        _scmap.setRangeLimits( mm);
        _scmap.rebuild();
    }   // end if
}   // end remove


// protected
void ScalarVisualisation::purge( FV* fv)
{
    remove(fv);
}   // end purge


// public
void ScalarVisualisation::updateFrom( const ColourMappingWidget* w)
{
    assert(w);
    QColor mcol0 = w->minColour();
    QColor mcol1 = w->midColour();
    QColor mcol2 = w->maxColour();
    _scmap.setColours( mcol0, mcol1, mcol2);
    _scmap.setNumColours( w->numColours());
    _scmap.setVisibleLimits( w->minScalar(), w->maxScalar());
    _scmap.rebuild();
}   // end updateFrom


// public
void ScalarVisualisation::updateTo( ColourMappingWidget* w) const
{
    assert(w);
    QColor c0, c1, c2;
    _scmap.colours(c0,c1,c2);
    const auto& rlims = _scmap.rangeLimits();
    const bool threeband = rlims.first < 0.0f && rlims.second > 0.0f;  // Will tell widget whether or not to hide "mid" colour

    w->setThreeBand( threeband);
    w->setMinColour(c0);
    w->setMidColour(c1);
    w->setMaxColour(c2);
    w->setScalarRangeLimits( rlims.first, rlims.second);
    w->setMinScalar( _scmap.minVisible());
    w->setMaxScalar( _scmap.maxVisible());
    w->setNumColours( _scmap.numColours());
}   // end updateTo
