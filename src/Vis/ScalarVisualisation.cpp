/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <vtkDataSetAttributes.h>
#include <vtkCellData.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
#include <PolySurfaceCurvScalarMapper.h>
using RFeatures::ObjModelCurvatureMetrics;
using FaceTools::Vis::LegendScalarColourRangeMapper;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::FaceView;
using FaceTools::Action::ChangeEventSet;
using FaceTools::FaceModelSurfaceData;
using FaceTools::SurfaceData;
using FaceTools::FaceControlSet;
using FaceTools::FaceViewerSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::FaceModelViewer;
using QTools::ColourMappingWidget;


ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i, const QKeySequence& k)
    : SurfaceVisualisation(d,i,k), _lrng( new LegendScalarColourRangeMapper( d.toStdString())), _mfunc(nullptr)
{
}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d, const QIcon& i)
    : SurfaceVisualisation(d,i), _lrng( new LegendScalarColourRangeMapper( d.toStdString())), _mfunc(nullptr)
{
}   // end ctor

ScalarVisualisation::ScalarVisualisation( const QString& d)
    : SurfaceVisualisation(d), _lrng( new LegendScalarColourRangeMapper( d.toStdString())), _mfunc(nullptr)
{
}   // end ctor


ScalarVisualisation::~ScalarVisualisation()
{
    delete _lrng;
}   // end dtor


namespace {
class CurvMapper : public RVTK::PolySurfaceCurvScalarMapper
{
public:
    CurvMapper( const FaceTools::ScalarMappingFunction& func, const ObjModelCurvatureMetrics* cm,
                vtkActor* actor, const IntIntMap* lookup, const std::string& mname)
        : RVTK::PolySurfaceCurvScalarMapper( cm, actor, lookup, mname), _func(func) {}
private:
    FaceTools::ScalarMappingFunction _func;
    float getCurvMetric( int faceIdx) const override { return _func( _cmetrics, faceIdx);}
};  // end class
}   // end namespace


// private
std::pair<float,float> ScalarVisualisation::mapActor( const FaceControl* fc) const
{
    SurfaceData::RPtr msd = FaceModelSurfaceData::rdata(fc->data()); // Scoped lock
    FaceView* fv = fc->view();
    assert( _mfunc != nullptr);
    CurvMapper mapper( _mfunc, msd->metrics, fv->surfaceActor(), &fv->polyLookups(), getDisplayName().toStdString());
    mapper.mapActor();
    float minval, maxval;
    mapper.getMappedRange( &minval, &maxval);
    return std::pair<float,float>( minval, maxval);
}   // end mapActor


// public
bool ScalarVisualisation::isAvailable( const FaceModel* fm) const
{
    return FaceModelSurfaceData::get()->isAvailable(fm) && _mfunc != nullptr;
}   // end isAvailable


// public
void ScalarVisualisation::apply( const FaceControl* fc)
{
    const std::string vname = getDisplayName().toStdString();

    // If the surface actor isn't yet mapped, or there's no _mapping entry
    // (because of purge), do the mapping of the surface actor.
    vtkActor* actor = fc->view()->surfaceActor();
    vtkDataSetAttributes* da = RVTK::getPolyData(actor)->GetCellData();
    if ( da->GetAbstractArray( vname.c_str()) == nullptr || _mappings.count(fc) == 0)
        _mappings[fc] = mapActor(fc);   // *** PROBABLE DEADLOCK BECAUSE OF DOUBLE READ LOCK in ActionVisualise and mapActor
    da->SetActiveScalars( vname.c_str());
    _added.insert(fc);
    remapColourRange();
    SurfaceVisualisation::apply(fc);
    actor->GetMapper()->SetScalarVisibility(true);
}   // end apply


// public
void ScalarVisualisation::addActors( const FaceControl* fc)
{
    SurfaceVisualisation::addActors(fc);
    vtkActor* actor = fc->view()->surfaceActor();
    FaceModelViewer* viewer = fc->viewer();
    _lrng->map( viewer, actor);
    viewer->showLegend(true);
}   // end addActors


// public
void ScalarVisualisation::removeActors( const FaceControl* fc)
{
    SurfaceVisualisation::removeActors(fc);
    FaceModelViewer* viewer = fc->viewer();

    const FaceControlSet& fcs = viewer->attached(); // All other views attached to the same viewer
    bool hideLegend = true;
    // If any of these have this visualisation applied, don't hide the legend
    for ( FaceControl* f : fcs)
    {
        assert(f->viewer() == viewer);
        if ( (f != fc) && isApplied(f))
        {
            hideLegend = false;
            break;
        }   // end if
    }   // end for

    _added.erase(fc);
    if ( hideLegend)
        viewer->showLegend(false);
    else
        remapColourRange();
}   // end removeActors


// protected
void ScalarVisualisation::purge( const FaceControl* fc)
{
    _added.erase(fc);
    _mappings.erase(fc);
    remapColourRange();
}   // end purge


// private
void ScalarVisualisation::remapColourRange()
{
    // Reset scalar range mapping to be the widest needed across all mappings.
    float nmin = FLT_MAX;
    float nmax = -FLT_MAX;
    for ( const auto& p : _mappings)
    {
        const std::pair<float,float>& mm = p.second;
        nmin = std::min<float>( nmin, mm.first);
        nmax = std::max<float>( nmax, mm.second);
    }   // end for
    _lrng->setRangeLimits( nmin, nmax);
    //std::cerr << "Legend set range min,max = " << nmin << ", " << nmax << std::endl;

    // Reset colour mappings for the actors.
    for ( const FaceControl* fc : _added)
        _lrng->map( fc->viewer(), fc->view()->surfaceActor());
}   // end remapColourRange


// public
bool ScalarVisualisation::updateColourMapping( const FaceControl* fc, const ColourMappingWidget* w)
{
    assert(w);
    assert(fc);
    if ( !isAvailable(fc->data()))
        return false;

    QColor mcol0 = w->minColour();
    QColor mcol1 = w->midColour();
    QColor mcol2 = w->maxColour();
    _lrng->setColours( mcol0, mcol1, mcol2);
    _lrng->setNumColours( w->numColours());
    _lrng->setVisibleRange( w->minScalar(), w->maxScalar());
    apply( fc);
    fc->viewer()->updateRender();
    return true;
}   // end doOnWidgetChanged


// public
bool ScalarVisualisation::updateWidget( ColourMappingWidget* w, const FaceControl* fc) const
{
    assert(w);
    assert(fc);
    if ( !isAvailable(fc->data()))
        return false;

    QColor c0, c1, c2;
    _lrng->colours(c0,c1,c2);
    const auto& vrng = _lrng->visibleRange();  // Currently set values.
    const auto& rlims = _lrng->rangeLimits();
    const bool threeband = rlims.first < 0.0f && rlims.second > 0.0f;  // Will tell widget whether or not to hide "mid" colour

    w->setThreeBand( threeband);
    w->setMinColour(c0);
    w->setMidColour(c1);
    w->setMaxColour(c2);
    w->setScalarRangeLimits( rlims.first, rlims.second);
    w->setMinScalar( vrng.first);
    w->setMaxScalar( vrng.second);
    w->setNumColours( _lrng->numColours());
    return true;
}   // end setWidgetTo

