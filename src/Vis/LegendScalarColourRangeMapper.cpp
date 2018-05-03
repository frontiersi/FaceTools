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

#include <LegendScalarColourRangeMapper.h>
#include <cassert>
using FaceTools::Vis::LegendScalarColourRangeMapper;
using FaceTools::ModelViewer;


LegendScalarColourRangeMapper::VisDeets::VisDeets()
    : ncols(100), range(0.0f,1.0f), lastr(0.0f,1.0f)
{
    cols[0] = cv::Vec3b(180,0,0);       // Blue
    cols[1] = cv::Vec3b(255,255,255);   // White
    cols[2] = cv::Vec3b(0,0,180);       // Red
}   // end ctor


// public
LegendScalarColourRangeMapper::LegendScalarColourRangeMapper( vtkActor* actor, bool amap)
    : _sactor(actor), _viewer(NULL), _smap(""), _autoRemap( amap)
{
    assert(actor);
}   // end ctor


// public
LegendScalarColourRangeMapper::~LegendScalarColourRangeMapper()
{
    setVisible();
}   // end dtor


// public
void LegendScalarColourRangeMapper::setVisible( std::string v, ModelViewer *viewer)
{
    if ( _viewer)
        _viewer->showLegend(false);
    _viewer = viewer;

    if ( v.empty()) // Use existing range name if empty
        v = _smap;

    if ( _vdeets.count(v) == 0)
    {
        assert(false);
        std::cerr << "[ERROR] FaceTools::Vis::LegendScalarColourRangeMapper::setVisible: Invalid range name passed in!" << std::endl;
        return;
    }   // end if

    _smap = v;
    if ( _viewer && !_smap.empty())
    {
        remap();
        _viewer->showLegend(true);
    }   // end if
}   // end setVisible


// public
bool LegendScalarColourRangeMapper::isVisible( const std::string& v) const
{
    bool possTrue = !_smap.empty();
    if ( !v.empty())
        possTrue &= _smap == v;
    return possTrue;
}   // end isVisible


// public
bool LegendScalarColourRangeMapper::gotMapping( const std::string& v) const { return _vdeets.count(v) > 0;}


// public
const std::pair<float,float>* LegendScalarColourRangeMapper::rangeLimits() const
{
    if ( !gotMapping(_smap))
        return NULL;
    return &_vdeets.at(_smap).range;
}   // end rangeLimits


// public
const std::pair<float,float>* LegendScalarColourRangeMapper::visibleRange() const
{
    if ( !gotMapping(_smap))
        return NULL;
    return &_vdeets.at(_smap).lastr;
}   // end visibleRange


// public
void LegendScalarColourRangeMapper::setRangeLimits( const std::string& v, float minv, float maxv)
{
    const std::pair<float,float> rng( minv, maxv);
    setRangeLimits( v, rng);
}   // end setRangeLimits


// public
void LegendScalarColourRangeMapper::setRangeLimits( const std::string& v, const std::pair<float,float>& rng)
{
    VisDeets vd;
    vd.range = rng;
    vd.lastr = rng;
    _vdeets[v] = vd;
}   // end setRangeLimits 


// public
void LegendScalarColourRangeMapper::changeVisibleMin( float dv)
{
    if ( _smap.empty())
        return;
    assert( gotMapping(_smap));
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);

    std::pair<float,float>& mm = vd.lastr;
    mm.first += dv; // Change min to be no higher than current max
    if ( mm.second < mm.first)
        mm.first = mm.second;
    const std::pair<float,float>& rmm = vd.range;
    if ( mm.first < rmm.first)      // And no lower than range min
        mm.first = rmm.first;
    if ( _autoRemap)
        remap();
}   // end changeVisibleMin


// public
void LegendScalarColourRangeMapper::changeVisibleMax( float dv)
{
    if ( _smap.empty())
        return;
    assert( gotMapping(_smap));
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    std::pair<float,float>& mm = vd.lastr;
    mm.second += dv; // Change max to be no lower than current min
    if ( mm.second < mm.first)
        mm.second = mm.first;
    const std::pair<float,float>& rmm = vd.range;
    if ( mm.second > rmm.second)    // And no higher than range max
        mm.second = rmm.second;
    if ( _autoRemap)
        remap();
}   // end changeVisibleMax


// public
void LegendScalarColourRangeMapper::setVisibleRange( float smin, float smax)
{
    if ( _smap.empty())
        return;
    assert( gotMapping(_smap));
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    std::pair<float,float>& mm = vd.lastr;
    mm.first = smin;
    mm.second = smax;
    // Bound within initial range
    const std::pair<float,float>& rmm = vd.range;
    if ( mm.first < rmm.first)
        mm.first = rmm.first;
    if ( mm.second > rmm.second)
        mm.second = rmm.second;
    if ( _autoRemap)
        remap();
}   // end setVisibleRange


// public
void LegendScalarColourRangeMapper::setNumColours( size_t nc)
{
    if ( !_viewer || _smap.empty())
        return;

    assert( gotMapping(_smap));
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    vd.ncols = std::max<size_t>(2, nc);
    if ( _autoRemap)
        remap();
}   // end setNumColours


// public
void LegendScalarColourRangeMapper::setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2)
{
    assert( _vdeets.count(_smap) > 0);
    if ( !_viewer || _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    vd.cols[0] = c0;
    vd.cols[1] = c1;
    vd.cols[2] = c2;
    if ( _autoRemap)
        remap();
}   // end setColours


void LegendScalarColourRangeMapper::setColours( const QColor& c0, const QColor& c1, const QColor& c2)
{
    const cv::Vec3b minCol( c0.red(), c0.green(), c0.blue());
    const cv::Vec3b midCol( c1.red(), c1.green(), c1.blue());
    const cv::Vec3b maxCol( c2.red(), c2.green(), c2.blue());
    setColours( minCol, midCol, maxCol);
}   // end setColours


void LegendScalarColourRangeMapper::colours( QColor& c0, QColor& c1, QColor& c2) const
{
    assert( _vdeets.count(_smap) > 0);
    if ( _vdeets.count(_smap) == 0)
        return;
    const VisDeets& vd = _vdeets.at(_smap);
    c0 = QColor( vd.cols[0][0], vd.cols[0][1], vd.cols[0][2]);
    c1 = QColor( vd.cols[1][0], vd.cols[1][1], vd.cols[1][2]);
    c2 = QColor( vd.cols[2][0], vd.cols[2][1], vd.cols[2][2]);
}   // end colours


size_t LegendScalarColourRangeMapper::numColours() const
{
    if ( _vdeets.count(_smap) == 0)
        return 0;
    return _vdeets.at(_smap).ncols;
}   // end numColours


// public
void LegendScalarColourRangeMapper::remap()
{
    assert( _vdeets.count(_smap) > 0);
    if ( !_viewer || _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    std::pair<float,float>& mm = vd.lastr;
    const float minv = mm.first;
    const float maxv = mm.second;
    _viewer->setLegendLookup( _sactor->GetMapper(), _smap, minv, maxv);

    if ( maxv < 0.0f)
        _viewer->setLegendColours( vd.cols[0], vd.cols[1], vd.ncols);
    else if ( minv > 0.0f)
        _viewer->setLegendColours( vd.cols[1], vd.cols[2], vd.ncols);
    else
    {
        // Calculate the proportional number of colours below zero (nc0) and above zero (nc1)
        const size_t nc0 = (size_t)(double(vd.ncols) * fabs(minv)/double(maxv - minv) + 0.5);
        const size_t nc1 = vd.ncols - nc0;
        _viewer->setLegendColours( vd.cols[0], vd.cols[1], vd.cols[2], nc0, nc1);
    }   // end else
}   // end remap
