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
 * Store and manage scalar colour mappings for a given vtkActor (Mapper).
 * Colours are mapped to the minimum, zero, and maximum values of a
 * scalar range. Used by ScalarVisualisation.
 */

#include <ModelViewer.h>
#include <unordered_map>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LegendScalarColourRangeMapper
{
public:
    LegendScalarColourRangeMapper( vtkActor*, bool autoRemap=true);
    virtual ~LegendScalarColourRangeMapper();

    // Set (and optionally select) a legend range mapping to be visible.
    // Legend is mapped to the viewer pointed at and is hidden from any
    // previous viewer. If the viewer parameter is left NULL, legend is
    // hidden from all viewers.
    // If the name is left empty, the existing mapping (last set) is used.
    // Note that if setting to be visible, then remapping of the visualisation
    // will occur whether or not auto-remapping was disabled in the constructor.
    void setVisible( std::string v="", ModelViewer* viewer=NULL);

    // Returns true if v is set to a non-empty string, matches a legend range,
    // and that legend range is currently set, or if v is empty, returns true
    // if ANY legend range is currently in the viewer. Returns false if no
    // legend is in the viewer, or (in the case that v is non-empty) false
    // if the specified legend range is not visible.
    bool isVisible( const std::string& v="") const;

    // Initialise mapped range to the given min, max bounds.
    void setRangeLimits( const std::string&, float minv, float maxv);
    void setRangeLimits( const std::string&, const std::pair<float,float>&);

    bool gotMapping( const std::string&) const;
    const std::string& rangeName() const { return _smap;}   // Current range name.
    const std::pair<float,float>* rangeLimits() const;      // Range of the currently set range (NULL if none set)
    const std::pair<float,float>* visibleRange() const;     // Min,max values of the current range (NULL if none set)

    // Change min/max mapping values by delta.
    void changeVisibleMin( float delta);
    void changeVisibleMax( float delta);
    
    // Set the visible range (won't set outside of bounded range).
    void setVisibleRange( float smin, float smax);

    // Set/get the colours to be used for the current range.
    void setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2);
    void setColours( const QColor& c0, const QColor& c1, const QColor& c2);
    void colours( QColor& c0, QColor& c1, QColor& c2) const;

    void setNumColours( size_t);
    size_t numColours() const;

    void remap();   // Force remapping to viewer for the current range (happens automatically if auto-remapping)

private:
    vtkActor *_sactor;
    ModelViewer *_viewer;   // The viewer (ScalarLegend) currently being mapped to.
    std::string _smap;      // Currently active visualisation
    bool _autoRemap;        // Automatically remap after any change?

    struct VisDeets
    {
        VisDeets();
        size_t ncols;                  // Number of colours
        std::pair<float,float> range;  // Allowed range
        std::pair<float,float> lastr;  // Current range
        cv::Vec3b cols[3];  // min, mid, and max colours
    };  // end struct

    std::unordered_map<std::string, VisDeets> _vdeets;
    LegendScalarColourRangeMapper( const LegendScalarColourRangeMapper&);   // No copy
    void operator=( const LegendScalarColourRangeMapper&);                  // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
