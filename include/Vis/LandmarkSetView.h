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

#ifndef FACE_TOOLS_LANDMARK_SET_VIEW_H
#define FACE_TOOLS_LANDMARK_SET_VIEW_H

/**
 * Handles the view of landmarks. Note that at any particular point in time, the apparent
 * position of the landmark may not reflect the actual position of the landmark.
 * In particular, function transform is used to adjust the apparent position of the
 * view objects representing the landmarks in the given LandmarkSet - it does NOT
 * change the ACTUAL position of the landmarks in the LandmarkSet! External clients
 * must coordinate updates of the view and the LandmarkSet.
 */

#include "SphereView.h"
#include <LandmarkSet.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LandmarkSetView
{
public:
    LandmarkSetView( LandmarkSet::Ptr, double defaultRadius=1.0);
    virtual ~LandmarkSetView();

    void setLandmarkRadius( double);
    double landmarkRadius() const { return _lmrad;}

    void setVisible( bool, ModelViewer*);               // Show/hide all landmarks (also sets current viewer)

    const std::unordered_set<int>& visible() const { return _visible;}  // IDs of visible landmarks
    void showLandmark( bool, int lmID);                 // Show/hide individual landmark

    const std::unordered_set<int>& highlighted() const { return _highlighted;} // IDs of highlighted landmarks
    void highlightLandmark( bool, int lmID);            // Highlights a (visible) landmark

    int landmark( const vtkProp*) const;                // Returns ID of landmark for prop or -1 if not found.

    // Refresh info about the given landmark from the set.
    // For lmids not in the set, the view is deleted. For lmids in the set, but not the view,
    // the landmark view is created. In all other cases, the landmark views are updated with
    // the current name and position of the landmark from the set.
    void updateLandmark( int lmID);

    // Refresh the visualised landmarks against what's in the stored LandmarkSet.
    void refresh();

    void pokeTransform( const vtkMatrix4x4*);                     
    void fixTransform();

private:
    LandmarkSet::Ptr _lset;
    double _lmrad;
    ModelViewer *_viewer;
    std::unordered_map<int, SphereView*> _views;
    std::unordered_map<const vtkProp*, int> _props; // Mapping of props to landmark IDs for fast lookup
    std::unordered_set<int> _highlighted;
    std::unordered_set<int> _visible;

    LandmarkSetView( const LandmarkSetView&) = delete;
    void operator=( const LandmarkSetView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
