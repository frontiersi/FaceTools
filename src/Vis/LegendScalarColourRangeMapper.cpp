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
#include <VtkTools.h>       // RVTK
#include <vtkProperty.h>
#include <vtkMapper.h>
using FaceTools::Vis::LegendScalarColourRangeMapper;
using FaceTools::ModelViewer;


// public
LegendScalarColourRangeMapper::LegendScalarColourRangeMapper( const std::string& smap)
    : _smap(smap), _ncols(100), _range(0.0f,1.0f), _lastr(0.0f,1.0f)
{
    _cols[0] = cv::Vec3b(180,0,0);       // Blue
    _cols[1] = cv::Vec3b(255,255,255);   // White
    _cols[2] = cv::Vec3b(0,0,180);       // Red
}   // end ctor


// public
void LegendScalarColourRangeMapper::setRangeLimits( float minv, float maxv) { setRangeLimits( std::pair<float,float>( minv, maxv));}
void LegendScalarColourRangeMapper::setRangeLimits( const std::pair<float,float>& rng) { _range = _lastr = rng; }


// public
void LegendScalarColourRangeMapper::setVisibleRange( float smin, float smax)
{
    std::pair<float,float>& mm = _lastr;
    mm.first = smin;
    mm.second = smax;
    // Bound within initial range
    const std::pair<float,float>& rmm = _range;
    if ( mm.first < rmm.first)
        mm.first = rmm.first;
    if ( mm.second > rmm.second)
        mm.second = rmm.second;
}   // end setVisibleRange


// public
void LegendScalarColourRangeMapper::setNumColours( size_t nc)
{
    _ncols = std::max<size_t>(2, nc);
}   // end setNumColours


// public
void LegendScalarColourRangeMapper::setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2)
{
    _cols[0] = c0;
    _cols[1] = c1;
    _cols[2] = c2;
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
    c0 = QColor( _cols[0][0], _cols[0][1], _cols[0][2]);
    c1 = QColor( _cols[1][0], _cols[1][1], _cols[1][2]);
    c2 = QColor( _cols[2][0], _cols[2][1], _cols[2][2]);
}   // end colours


// public
void LegendScalarColourRangeMapper::map( ModelViewer *viewer, vtkActor *actor) const
{
    const float minv = _lastr.first;
    const float maxv = _lastr.second;

    size_t nc0 = _ncols;
    size_t nc1 = _ncols;
    if ( maxv <= 0.0f)
        nc1 = 0;    // Will use upper (positive) colour range only
    else if ( minv >= 0.0f)
        nc0 = 0;    // Will use lower (negative) colour range only
    else
    {   // Calculate the proportional number of colours below zero (nc0) and above zero (nc1)
        nc0 = (size_t)(double(_ncols) * fabs(minv)/double(maxv - minv) + 0.5);
        nc1 = _ncols - nc0;
    }   // end else

    if ( nc1 == 0)
        viewer->setLegendColours( _cols[0], _cols[1], nc0);
    else if ( nc0 == 0)
        viewer->setLegendColours( _cols[1], _cols[2], nc1);
    else
        viewer->setLegendColours( _cols[0], _cols[1], _cols[2], nc0, nc1);

    viewer->setLegendLookup( actor->GetMapper(), _smap, minv, maxv);
}   // end map

