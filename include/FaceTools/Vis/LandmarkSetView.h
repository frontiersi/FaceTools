/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
#include <FaceTools/LndMrk/LandmarkSet.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LandmarkSetView
{
public:
    explicit LandmarkSetView( double radius=1.2);
    virtual ~LandmarkSetView();

    void setLandmarkRadius( double);
    double landmarkRadius() const { return _lmrad;}

    void setSelectedColour( bool);

    void setVisible( bool, ModelViewer*);
    bool isVisible() const { return _visible;}

    void showLandmark( bool, int lmID);

    void setLabelVisible( bool, int, FaceSide);
    void setHighlighted( bool, int, FaceSide);

    // Set the untransformed landmark position.
    void set( int, FaceSide, const Vec3f&);
    void remove( int);

    // Returns ID of landmark for prop or -1 if not found.
    // On return >= 0, out parameter FaceSide is set to the
    // lateral on which the landmark appears.
    int landmarkId( const vtkProp*, FaceSide&) const;

    void pokeTransform( const vtkMatrix4x4*);

private:
    double _lmrad;
    ModelViewer *_viewer;
    bool _visible;

    using SphereMap = std::unordered_map<int, SphereView*>;
    using PropMap = std::unordered_map<const vtkProp*, int>;

    SphereMap _lviews, _mviews, _rviews;
    PropMap _lprops, _mprops, _rprops;

    void _remove( int, SphereMap&, PropMap&);
    void _set( int, FaceSide, SphereMap&, PropMap&, const Vec3f&);
    void _setLandmarkColour( const Vec3f&, int, FaceSide);
    LandmarkSetView( const LandmarkSetView&) = delete;
    void operator=( const LandmarkSetView&) = delete;
};  // end class

}}   // end namespace

#endif
