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

#ifndef FACE_TOOLS_LANDMARK_GROUP_VIEW_H
#define FACE_TOOLS_LANDMARK_GROUP_VIEW_H

#include "ObjMetaData.h"
#include "LandmarkView.h"
#include "ModelOptions.h"

namespace FaceTools
{

class FaceTools_EXPORT LandmarkGroupView
{
public:
    LandmarkGroupView( const ObjMetaData::Ptr);
    virtual ~LandmarkGroupView();

    void setVisible( bool, ModelViewer*);        // Show/hide all landmarks (also sets current viewer)
    bool isVisible() const;                      // Returns true iff ANY landmark shown
    void getVisibleLandmarks( std::vector<std::string>&) const; // Get the names of the visible landmarks into the given vector.

    void showLandmark( bool, const std::string&);       // Show/hide individual landmark
    bool isLandmarkVisible( const std::string&) const;  // Returns true iff a particular landmark is shown
    void highlightLandmark( bool, const std::string&);  // Highlights a (visible) landmark

    void setOptions( const ModelOptions::Landmarks&);

    void erase(); // Remove existing landmarks
    void reset(); // Reset all landmarks from the ObjMetaData

    // Returns the name of the landmark under the given coordinates
    // or an empty string if no landmarks lie under the coordinates.
    std::string pointedAt( const QPoint&) const;

    // Returns true if given prop is one of the landmarks AND the landmark is visible.
    bool isLandmark( const vtkProp*) const;

    void updateLandmark( const std::string&, const cv::Vec3f*);

private:
    ModelViewer *_viewer;
    const ObjMetaData::Ptr _omd;
    boost::unordered_map<std::string, LandmarkView*> _lviews;
    ModelOptions::Landmarks _opts;

    LandmarkGroupView( const LandmarkGroupView&);   // No copy
    void operator=( const LandmarkGroupView&);      // No copy
};  // end class

}   // end namespace

#endif
