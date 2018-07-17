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

#ifndef FACE_TOOLS_PATH_VIEW_H
#define FACE_TOOLS_PATH_VIEW_H

/**
 * Represents the visualisation of path data between two points on a FaceModel.
 */

#include "SphereView.h"
#include <Path.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT PathView
{
public:
    // Provide the Path instance that this view will be referencing for updates.
    explicit PathView( const Path&);
    virtual ~PathView();

    const Path& path() const { return _path;}

    bool isVisible() const { return _linePropID >= 0;}
    void setVisible( bool, ModelViewer*);  // Add/remove the path actors (handles and path) to the viewer.

    // Update the path actors from the internal Path data.
    // Call whenever the internally referenced Path instance is changed.
    // Path should already be visible (if not, this function makes it so).
    void update();

    struct FaceTools_EXPORT Handle
    {
        cv::Vec3f pos() const { return _sv->centre();}
        void highlight(bool v) { _sv->highlight(v);}
        bool isHighlighted() const { return _sv->isHighlighted();}
        PathView* host() const { return _host;}
        const vtkProp* prop() { return _sv->prop();}

    private:
        Handle( PathView*, const cv::Vec3f&, double);
        ~Handle();
        PathView *_host;
        SphereView *_sv;
        friend class PathView;
    };  // end struct

    Handle* handle0() const { return _h0;}
    Handle* handle1() const { return _h1;}

    void pokeTransform( const vtkMatrix4x4*);
    void fixTransform();

private:
    ModelViewer *_viewer;
    const Path& _path;
    Handle *_h0, *_h1;
    int _linePropID;

    void addPath();
    void removePath();
    PathView( const PathView&);         // No copy
    void operator=( const PathView&);   // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
