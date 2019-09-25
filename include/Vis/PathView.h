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

#ifndef FACE_TOOLS_PATH_VIEW_H
#define FACE_TOOLS_PATH_VIEW_H

#include "SphereView.h"
#include <ModelViewer.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathView
{
public:
    PathView( int id, const std::list<cv::Vec3f>& vtxs);
    virtual ~PathView();

    // Call whenever given Path instance is changed (id of given path must match internal).
    void update( const std::list<cv::Vec3f>&);

    int id() const { return _id;}
    void setVisible( bool, ModelViewer*);  // Add/remove the path actors (handles and path) to the viewer.

    struct FaceTools_EXPORT Handle
    {
        int handleId() const { return _hid;}
        int pathId() const { return _pid;}
        cv::Vec3f viewPos() const;  // Position handle is being viewed at (after applying view transform).
        const cv::Vec3f& realPos() const { return _sv->centre();}  // "Real" position of handle (no view transform applied).
        const vtkProp* prop() { return _sv->prop();}
        void setCaption( const QString& c) { _sv->setCaption(c);}
        void showCaption( bool v) { _sv->setHighlighted(v);}

    private:
        Handle( int, int, const cv::Vec3f&, double);
        ~Handle();
        Handle( const Handle&) = delete;
        void operator=( const Handle&) = delete;
        int _hid;
        int _pid;
        SphereView *_sv;
        friend class PathView;
    };  // end struct

    Handle* handle0() { return _h0;}
    Handle* handle1() { return _h1;}

    void updateColours();

    void pokeTransform( const vtkMatrix4x4*);

private:
    ModelViewer *_viewer;
    int _id;
    Handle *_h0, *_h1;
    vtkSmartPointer<vtkActor> _lprop;
#ifndef NDEBUG
    vtkSmartPointer<vtkActor> _pprop;
#endif

    PathView( const PathView&) = delete;
    void operator=( const PathView&) = delete;
};  // end class

}}   // end namespaces

#endif
