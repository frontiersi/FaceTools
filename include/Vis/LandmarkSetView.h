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

#include "LandmarkView.h"
#include <LandmarkSet.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT LandmarkSetView
{
public:
    LandmarkSetView( const LandmarkSet&);
    virtual ~LandmarkSetView();

    void erase();                                       // Remove existing landmark views.
    void reset();                                       // Reset all landmark views from the set.

    bool isVisible() const;                             // Returns true iff ANY landmark shown
    void setVisible( bool, ModelViewer*);               // Show/hide all landmarks (also sets current viewer)

    void showLandmark( bool, int lmID);                 // Show/hide individual landmark
    bool isLandmarkVisible( int lmID) const;            // Returns true iff a particular landmark is shown
    const std::unordered_set<int>& visible() const;     // IDs of the visible landmarks

    void highlightLandmark( bool, int lmID);            // Highlights a (visible) landmark
    bool isLandmarkHighlighted( int lmID) const;        // Returns true iff a particular landmark is highlighted
    const std::unordered_set<int>& highlighted() const; // IDs of the highlighted landmarks

    int pointedAt( const QPoint&) const;                // Returns ID of the landmark under the given coordinates or -1 if none.
    bool isLandmark( const vtkProp*) const;             // Returns true if given prop is a landmark AND the landmark is visible.
    const LandmarkView* landmark( int lmID) const;      // Return the landmark view with this ID or NULL if not found.

    // Transform the landmark views according to the given matrix.
    // This function does NOT change the real position of the landmark!
    void transform( const vtkMatrix4x4*);                     

    // Refresh info about the given landmark from the set.
    // For lmids not in the set, the view is deleted. For lmids in the set, but not the view,
    // the landmark view is created. In all other cases, the landmark views are updated with
    // the current name and position of the landmark from the set.
    void refreshLandmark( int lmid);

private:
    const LandmarkSet& _lset;
    ModelViewer *_viewer;
    std::unordered_map<int, LandmarkView*> _lviews;
    std::unordered_set<int> _visible, _highlighted;

    LandmarkSetView( const LandmarkSetView&);   // No copy
    void operator=( const LandmarkSetView&);    // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
