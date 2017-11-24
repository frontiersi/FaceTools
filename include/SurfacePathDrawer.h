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

#ifndef FACE_TOOLS_SURFACE_PATH_DRAWER_H
#define FACE_TOOLS_SURFACE_PATH_DRAWER_H

#include "FaceControl.h"
#include <vtkSphereSource.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <QPoint>
#include <QColor>

namespace FaceTools
{

class FaceTools_EXPORT SurfacePathDrawer
{
public:
    SurfacePathDrawer( FaceControl*, const std::string& modelUnits="mm", const QColor& textColour=Qt::white, bool showCaptions=true);
    virtual ~SurfacePathDrawer();

    void clearHandles();     // Unset the endpoints
    size_t nhandles() const; // Number of endpoint handles set (0, 1, or 2).
    void setUnits( const std::string&);
    void setVisible( bool);    // Show or hide path information
    bool isVisible() const;

    struct FaceTools_EXPORT Handle
    {
        void set( const QPoint&);   // Update the position of this endpoint handle.
        cv::Vec3f get() const;
        void highlight(bool);
    private:
        explicit Handle( SurfacePathDrawer*);
        SurfacePathDrawer* _host;
        vtkSmartPointer<vtkSphereSource> _source;
        vtkSmartPointer<vtkActor> _actor;
        friend class SurfacePathDrawer;
    };  // end struct

    // Set positions of the handles without needing a reference to them
    // Both functions return pointers to the handles after setting.
    Handle* setHandle0( const QPoint&);
    Handle* setHandle1( const QPoint&);

    // Highlight the specified handles.
    void highlightHandle0( bool);
    void highlightHandle1( bool);

    // Returns the handle corresponding to a given display
    // coordinate or NULL if no handle under the coordinate.
    Handle* handle( const QPoint&) const;

private:
    FaceControl* _fcont;
    Handle *_h0, *_h1;
    std::string _munits;
    bool _visible;
    int _pathID;
    vtkSmartPointer<vtkTextActor> _lenText;
    vtkSmartPointer<vtkCaptionActor2D> _lenCaption;

    void addPath();
    void removePath();
    bool createPath( std::vector<cv::Vec3f>&);
    SurfacePathDrawer( const SurfacePathDrawer&); // NO COPY
    void operator=( const SurfacePathDrawer&);    // NO COPY
};  // end class

}   // end namespace

#endif


