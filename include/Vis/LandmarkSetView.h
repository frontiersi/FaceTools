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
    LandmarkSetView( const LandmarkSet&, double lmrad=1.5);
    virtual ~LandmarkSetView();

    bool isVisible() const;                             // Returns true iff ANY landmark shown
    void setVisible( bool, ModelViewer*);               // Show/hide all landmarks (also sets current viewer)

    void showLandmark( bool, int lmID);                 // Show/hide individual landmark
    bool isLandmarkVisible( int lmID) const;            // Returns true iff a particular landmark is shown
    const std::unordered_set<int>& visible() const;     // IDs of the visible landmarks

    void highlightLandmark( bool, int lmID);            // Highlights a (visible) landmark
    bool isLandmarkHighlighted( int lmID) const;        // Returns true iff a particular landmark is highlighted
    const std::unordered_set<int>& highlighted() const; // IDs of the highlighted landmarks

    void setLandmarkRadius( double);
    double landmarkRadius() const { return _lmrad;}

    int pointedAt( const QPoint&) const;                // Returns ID of landmark under the given coordinates or -1 if none.
    bool isLandmark( const vtkProp*) const;             // Returns true if given prop is landmark AND landmark is visible.
    const SphereView* landmark( int lmID) const;        // Return the landmark view with this ID or NULL if not found.

    void pokeTransform( const vtkMatrix4x4*);                     
    void fixTransform();

    // Refresh info about the given landmark from the set.
    // For lmids not in the set, the view is deleted. For lmids in the set, but not the view,
    // the landmark view is created. In all other cases, the landmark views are updated with
    // the current name and position of the landmark from the set.
    void refreshLandmark( int lmid);

private:
    const LandmarkSet& _lset;
    double _lmrad;
    ModelViewer *_viewer;
    std::unordered_map<int, SphereView*> _lviews;
    std::unordered_map<const vtkProp*, int> _props; // Mapping of props to landmark IDs for fast lookup
    std::unordered_set<int> _highlighted;
    std::unordered_set<int> _visible;

    LandmarkSetView( const LandmarkSetView&) = delete;
    void operator=( const LandmarkSetView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
