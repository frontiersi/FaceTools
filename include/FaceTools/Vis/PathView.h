/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include "AngleView.h"
#include <FaceTools/Path.h>
#include <FaceTools/ModelViewer.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathView
{
public:
    explicit PathView( int id);
    virtual ~PathView();

    // Call whenever given Path instance is changed (id of given path must match internal).
    void update( const Path &path);

    int id() const { return _id;}
    void setVisible( bool, ModelViewer*);  // Add/remove the path actors (handles and path) to the viewer.

    struct FaceTools_EXPORT Handle
    {
        int handleId() const { return _hid;}
        int pathId() const { return _pid;}
        Vec3f viewPos() const;  // Position handle is being viewed at (after applying view transform).
        const vtkProp* prop() const { return _sv->prop();}
        float radius() const { return _sv->radius();}
        void setCaption( const QString& c) { _sv->setCaption(c);}
        void showCaption( bool v) { _sv->showCaption(v);}

    private:
        Handle( int, int, const Vec3f&, double);
        ~Handle();
        Handle( const Handle&) = delete;
        void operator=( const Handle&) = delete;
        int _hid;
        int _pid;
        SphereView *_sv;
        friend class PathView;
    };  // end struct

    Handle* handle( int hid);
    Handle* handle0() { return _h0;}
    Handle* handle1() { return _h1;}
    Handle* depthHandle() { return _g;}

    void updateColours();

    void setPickable( bool);

    void pokeTransform( const vtkMatrix4x4*);

private:
    ModelViewer *_viewer;
    bool _isVisible;
    bool _hasSurface;
    int _id;
    Handle *_h0, *_h1;
    Handle *_g;     // Depth changing handle
    vtkSmartPointer<vtkActor> _sprop;   // Surface path
    vtkSmartPointer<vtkActor> _hprop;   // Direct line between handles
    vtkSmartPointer<vtkActor> _jprop;   // Direct line between pseudo handles
    vtkSmartPointer<vtkActor> _p0prop;  // Caliper "pincer" 0
    vtkSmartPointer<vtkActor> _p1prop;  // Caliper "pincer" 1
    vtkSmartPointer<vtkActor> _dprop;   // Depth (plump) line at deepest part
    AngleView _aview;

    void _addLineProps();
    void _removeLineProps();

    PathView( const PathView&) = delete;
    void operator=( const PathView&) = delete;
};  // end class

}}   // end namespaces

#endif
