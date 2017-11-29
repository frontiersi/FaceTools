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

#include <LegendRange.h>
using FaceTools::LegendRange;
using FaceTools::ModelViewer;


LegendRange::VisDeets::VisDeets()
    : ncols(100), range(0.0f,1.0f), lastr(0.0f,1.0f)
{
    cols[0] = cv::Vec3b(180,0,0);
    cols[1] = cv::Vec3b(255,255,255);
    cols[2] = cv::Vec3b(0,0,180);
}   // end ctor


// public
LegendRange::LegendRange() : _viewer(NULL), _sactor(NULL), _smap("")
{
}   // end ctor


// public
LegendRange::~LegendRange()
{
    setVisible();   // Default is to remove from viewer
}   // end dtor


// public
void LegendRange::setActor( vtkActor* actor)
{
    _sactor = actor;
}   // end setActor


// public
void LegendRange::setVisible( std::string v, ModelViewer* viewer)
{
    if ( _viewer)
        _viewer->showLegend(false);

    if ( !viewer)
        viewer = _viewer;
    else
        viewer->showLegend(false);

    restoreRange("");
    _viewer = viewer;
    if ( !v.empty() && viewer && _sactor)
    {
        restoreRange(v);
        viewer->showLegend(true);
    }   // end if
}   // end setVisible


// public
bool LegendRange::isVisible( const std::string& v) const
{
    bool possTrue = _viewer && !_smap.empty();
    if ( !v.empty())
        possTrue &= _smap == v;
    return possTrue;
}   // end isVisible


// public
bool LegendRange::gotMapping( const std::string& v) const { return _vdeets.count(v) > 0;}


// private
bool LegendRange::restoreRange( const std::string& v)
{
    _smap = "";
    if ( v == "")
        return true;

    assert( gotMapping(v));
    if ( _vdeets.count(v) == 0)
        return false;

    _smap = v;
    if ( _viewer && _sactor)
    {
        const VisDeets& vd = _vdeets.at(v);
        const std::pair<float,float>& mm = vd.lastr;
        const float minv = mm.first;
        const float maxv = mm.second;
        _viewer->setLegendLookup( _sactor->GetMapper(), v, minv, maxv);
        setColours( vd.cols[0], vd.cols[1], vd.cols[2]);
        setNumColours( vd.ncols);
    }   // end if
    return true;
}   // end restoreRange


// public
const std::pair<float,float>* LegendRange::getRange() const
{
    if ( !gotMapping(_smap))
        return NULL;
    return &_vdeets.at(_smap).range;
}   // end getRange


// public
const std::pair<float,float>* LegendRange::getCurrent() const
{
    if ( !gotMapping(_smap))
        return NULL;
    return &_vdeets.at(_smap).lastr;
}   // end getCurrent


// public
void LegendRange::setBounds( const std::string& v, float minv, float maxv)
{
    const std::pair<float,float> rng( minv, maxv);
    setBounds( v, rng);
}   // end setBounds


// public
void LegendRange::setBounds( const std::string& v, const std::pair<float,float>& rng)
{
    VisDeets vd;
    vd.range = rng;
    vd.lastr = rng;
    _vdeets[v] = vd;
}   // end setBounds 


// public
void LegendRange::changeScalarMappingMin( float dv)
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
    std::string v = _smap;
    restoreRange( v);
}   // end changeScalarMappingMin


// public
void LegendRange::changeScalarMappingMax( float dv)
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
    std::string v = _smap;
    restoreRange( v);
}   // end changeScalarMappingMax


// public
void LegendRange::setScalarMappingMinMax( float smin, float smax)
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
    std::string v = _smap;
    restoreRange( v);
}   // end setScalarMappingMinMax


// public
void LegendRange::setNumColours( int nc)
{
    if ( _smap.empty())
        return;

    assert( _viewer);
    assert( gotMapping(_smap));
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    vd.ncols = std::max<int>(2, nc);

    std::pair<float,float>& mm = vd.lastr;
    const float minv = mm.first;
    const float maxv = mm.second;
    if ( maxv < 0.0f)
        _viewer->setLegendColours( vd.cols[0], vd.cols[1], vd.ncols);
    else if ( minv > 0.0f)
        _viewer->setLegendColours( vd.cols[1], vd.cols[2], vd.ncols);
    else
    {
        const int nc0 = (int)(double(vd.ncols) * fabs(minv)/double(maxv - minv) + 0.5);
        const int nc1 = vd.ncols - nc0;
        _viewer->setLegendColours( vd.cols[0], vd.cols[1], vd.cols[2], nc0, nc1);
    }   // end else
}   // end setNumColours


// public
void LegendRange::setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2)
{
    assert( _vdeets.count(_smap) > 0);
    if ( _vdeets.count(_smap) == 0)
        return;

    VisDeets& vd = _vdeets.at(_smap);
    vd.cols[0] = c0;
    vd.cols[1] = c1;
    vd.cols[2] = c2;
    if ( _viewer)
    {
        const int nc = vd.ncols;
        const int hnc = nc/2;
        _viewer->setLegendColours( c0, c1, c2, hnc, nc-hnc);
    }   // end if
}   // end setColours


// public
void LegendRange::setColours( const QColor& c0, const QColor& c1, const QColor& c2)
{
    const cv::Vec3b minCol( c0.red(), c0.green(), c0.blue());
    const cv::Vec3b midCol( c1.red(), c1.green(), c1.blue());
    const cv::Vec3b maxCol( c2.red(), c2.green(), c2.blue());
    setColours( minCol, midCol, maxCol);
}   // end setColours


// public
void LegendRange::getColours( QColor& c0, QColor& c1, QColor& c2) const
{
    assert( _vdeets.count(_smap) > 0);
    if ( _vdeets.count(_smap) == 0)
        return;
    const VisDeets& vd = _vdeets.at(_smap);
    c0 = QColor( vd.cols[0][0], vd.cols[0][1], vd.cols[0][2]);
    c1 = QColor( vd.cols[1][0], vd.cols[1][1], vd.cols[1][2]);
    c2 = QColor( vd.cols[2][0], vd.cols[2][1], vd.cols[2][2]);
}   // end getColours


// public
int LegendRange::getNumColours() const
{
    assert( _vdeets.count(_smap) > 0);
    if ( _vdeets.count(_smap) == 0)
        return 0;
    return _vdeets.at(_smap).ncols;
}   // end getNumColours
