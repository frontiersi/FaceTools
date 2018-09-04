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

#ifndef FACE_TOOLS_PATH_VIEW_H
#define FACE_TOOLS_PATH_VIEW_H

#include "SphereView.h"
#include <ModelViewer.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT PathView
{
public:
    PathView( int id, const std::vector<cv::Vec3f>& vtxs);
    virtual ~PathView();

    // Call whenever given Path instance is changed (id of given path must match internal).
    void update( const std::vector<cv::Vec3f>&);

    int id() const { return _id;}
    void setVisible( bool, ModelViewer*);  // Add/remove the path actors (handles and path) to the viewer.

    struct FaceTools_EXPORT Handle
    {
        int handleId() const { return _hid;}
        int pathId() const { return _pid;}
        cv::Vec3f pos() const { return _sv->centre();}
        const vtkProp* prop() { return _sv->prop();}

    private:
        Handle( int, int, const cv::Vec3f&, double);
        ~Handle();
        int _hid;
        int _pid;
        SphereView *_sv;
        friend class PathView;
    };  // end struct

    Handle* handle0() const { return _h0;}
    Handle* handle1() const { return _h1;}

    void pokeTransform( const vtkMatrix4x4*);
    void fixTransform();

private:
    ModelViewer *_viewer;
    int _id;
    Handle *_h0, *_h1;
    vtkActor* _lprop;

    PathView( const PathView&) = delete;
    void operator=( const PathView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
