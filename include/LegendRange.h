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

#ifndef FACE_TOOLS_LEGEND_RANGE_H
#define FACE_TOOLS_LEGEND_RANGE_H

/**
 * Managing and storing changes to the scalar legend for different visualisations.
 */

#include "ModelViewer.h"
#include <map>  // std::pair

namespace FaceTools
{

class FaceTools_EXPORT LegendRange
{
public:
    LegendRange();
    virtual ~LegendRange();

    void setActor( vtkActor*);

    // Set a new viewer to map to and the same time as restoring a range.
    // If the range name is left empty, the existing range is used.
    // If the viewer is NULL, the existing viewer is used.
    void setVisible( std::string v="", ModelViewer* viewer=NULL);

    // Returns true if v is set to a non-empty string, matches a legend range,
    // and that legend range is currently set, or if v is empty, returns true
    // if ANY legend range is currently in the viewer. Returns false if no
    // legend is in the viewer, or (in the case that v is non-empty) false
    // if the specified legend range is not visible.
    bool isVisible( const std::string& v="") const;

    // Initialise mapped range to the given min, max bounds.
    void setBounds( const std::string&, float minv, float maxv);
    void setBounds( const std::string&, const std::pair<float,float>&);

    bool gotMapping( const std::string&) const;
    const std::string& getTitle() const { return _smap;}    // Title of the current range
    const std::pair<float,float>* getRange() const;         // Range of the currently set range (NULL if none set)
    const std::pair<float,float>* getCurrent() const;       // Min,max values of the current range (NULL if none set)

    // Change min/max mapping values by delta.
    void changeScalarMappingMin( float delta);
    void changeScalarMappingMax( float delta);
    
    // Set the visible range (won't set outside of bounded range).
    void setScalarMappingMinMax( float smin, float smax);

    // Set/get the colours to be used for the current range.
    void setColours( const cv::Vec3b& c0, const cv::Vec3b& c1, const cv::Vec3b& c2);
    void setColours( const QColor& c0, const QColor& c1, const QColor& c2);
    void getColours( QColor& c0, QColor& c1, QColor& c2) const;

    // Only call after having called restoreRange.
    void setNumColours( int nc);
    int getNumColours() const;

private:
    ModelViewer *_viewer;
    vtkActor* _sactor;
    std::string _smap;  // Currently active visualisation

    struct VisDeets
    {
        VisDeets();
        int ncols;                     // Number of colours
        std::pair<float,float> range;  // Allowed range
        std::pair<float,float> lastr;  // Current range
        cv::Vec3b cols[3];  // min, mid, and max colours
    };  // end struct

    bool restoreRange( const std::string &v="");         // Returns true if able to restore
    boost::unordered_map<std::string, VisDeets> _vdeets;

    LegendRange( const LegendRange&);       // No copy
    void operator=( const LegendRange&);    // No copy
};  // end class

}   // end namespace

#endif

