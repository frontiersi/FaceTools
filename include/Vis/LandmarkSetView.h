/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include "SphereView.h"
#include <LndMrk/LandmarkSet.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LandmarkSetView
{
public:
    explicit LandmarkSetView( double radius=1.2);
    virtual ~LandmarkSetView();

    void setLandmarkRadius( double);
    double landmarkRadius() const { return _lmrad;}

    void setColour( const cv::Vec3d&);

    void setVisible( bool, ModelViewer*);              // Show/hide all landmarks (also sets current viewer)

    const IntSet& visible() const { return _visible;}  // IDs of visible landmarks
    void showLandmark( bool, int lmID);                // Show/hide individual landmark

    const IntSet& highlighted() const { return _highlighted;} // IDs of highlighted landmarks
    void highlightLandmark( bool, int, FaceLateral);          // Highlights a (visible) landmark

    // Set the landmark position with the model in its current position.
    // The matrix is the transform applied to the current model.
    void set( int, FaceLateral, const cv::Vec3f&, const vtkMatrix4x4*);
    void remove( int);

    // Returns ID of landmark for prop or -1 if not found.
    // On return >= 0, out parameter FaceLateral is set to the
    // lateral on which the landmark appears.
    int landmarkId( const vtkProp*, FaceLateral&) const;

    void pokeTransform( const vtkMatrix4x4*);

    static cv::Vec3d BASE0_COL;
    static cv::Vec3d SPEC0_COL;

private:
    double _lmrad;
    ModelViewer *_viewer;
    using SphereMap = std::unordered_map<int, SphereView*>;
    SphereMap _lviews, _mviews, _rviews;
    using PropMap = std::unordered_map<const vtkProp*, int>;
    PropMap _lprops, _mprops, _rprops;
    IntSet _highlighted;
    IntSet _visible;

    void _remove( int, SphereMap&, PropMap&);
    void _set( int, FaceLateral, SphereMap&, PropMap&, const cv::Vec3f&, const vtkMatrix4x4*);
    LandmarkSetView( const LandmarkSetView&) = delete;
    void operator=( const LandmarkSetView&) = delete;
};  // end class

}}   // end namespace

#endif
