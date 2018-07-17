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

#ifndef FACE_TOOLS_VIS_LEGEND_SCALAR_COLOUR_RANGE_MAPPER_H
#define FACE_TOOLS_VIS_LEGEND_SCALAR_COLOUR_RANGE_MAPPER_H

/**
 * Store and manage scalar colour mappings for a surface mapping.
 * To apply the mappings, pass in a valid viewer and actor to the map function.
 * Colours are mapped to the minimum, zero, and maximum values of a scalar range.
 */

#include <ModelViewer.h>
#include <unordered_map>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LegendScalarColourRangeMapper
{
public:
    explicit LegendScalarColourRangeMapper( const std::string& smap);

    const std::string& rangeName() const { return _smap;}

    // Set/get the min/max bounds of the mapping range.
    void setRangeLimits( float minv, float maxv);
    void setRangeLimits( const std::pair<float,float>&);
    const std::pair<float,float>& rangeLimits() const { return _range;}
    const std::pair<float,float>& visibleRange() const { return _lastr;}

    // Set the visible range (won't set outside of bounded range).
    void setVisibleRange( float smin, float smax);

    // Set/get the colours to be used for the current range.
    void setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2);
    void setColours( const QColor& c0, const QColor& c1, const QColor& c2);
    void colours( QColor& c0, QColor& c1, QColor& c2) const;

    void setNumColours( size_t);
    size_t numColours() const { return _ncols;}

    void map( ModelViewer*, vtkActor*) const;

private:
    const std::string _smap;
    size_t _ncols;                  // Number of colours
    std::pair<float,float> _range;  // Allowed range
    std::pair<float,float> _lastr;  // Current range
    cv::Vec3b _cols[3];  // min, mid, and max colours

    LegendScalarColourRangeMapper( const LegendScalarColourRangeMapper&);   // No copy
    void operator=( const LegendScalarColourRangeMapper&);                  // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
