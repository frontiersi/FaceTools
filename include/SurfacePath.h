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

#ifndef FACE_TOOLS_SURFACE_PATH_H
#define FACE_TOOLS_SURFACE_PATH_H

#include "FaceControl.h"
#include <ObjModelSurfacePathFinder.h>  // RFeatures
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <QPoint>
#include <QColor>

namespace FaceTools {

class FaceTools_EXPORT SurfacePath
{
public:
    SurfacePath( const FaceControl*, const QPoint&);    // Both handles initially set to given point
    virtual ~SurfacePath();

    // Provide a different surface path finder delegate if default not suitable (may be slow - doesn't use kd tree).
    void setSurfacePathFinder( const RFeatures::ObjModelSurfacePathFinder& sp) { _spfinder = sp;}

    void setVisible( bool);  // Add/remove the path actors (handles and path) to the FaceControl's viewer.
    bool isVisible() const;

    struct FaceTools_EXPORT Handle
    {
        void set( const QPoint&);   // Set the position of this endpoint handle.
        cv::Vec3f pos() const;      // 3D position in space.
        void highlight(bool);
        bool isHighlighted() const { return _highlighted;}
        SurfacePath* host() const { return _host;}
        const vtkSmartPointer<vtkProp> actor() { return _actor;}
    private:
        explicit Handle( SurfacePath*);
        SurfacePath* _host;
        vtkSmartPointer<vtkSphereSource> _source;
        vtkSmartPointer<vtkActor> _actor;
        bool _highlighted;
        friend class SurfacePath;
    };  // end struct

    Handle* handle0() const { return _h0;}
    Handle* handle1() const { return _h1;}
    Handle* handle( const QPoint&) const; // Handle corresponding to given display coords or NULL if no handle under coords.

private:
    const FaceControl* _fcont;
    Handle *_h0, *_h1;
    bool _visible;
    int _pathID;
    RFeatures::ObjModelSurfacePathFinder _spfinder;
    vtkSmartPointer<vtkTextActor> _lenText;
    vtkSmartPointer<vtkCaptionActor2D> _lenCaption;

    void addPath();
    void removePath();
    bool createPath();
    SurfacePath( const SurfacePath&);   // No copy
    void operator=( const SurfacePath&);// No copy
};  // end class

}   // end namespace

#endif
