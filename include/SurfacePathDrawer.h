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

#include "ObjMetaData.h"
#include <ModelPathDrawer.h>    // RVTK
#include <vtkRenderWindow.h>
#include <vtkCaptionActor2D.h>
#include <vtkTextActor.h>
#include <QObject>
#include <QPoint>

namespace FaceTools
{

class ModelViewer;

class FaceTools_EXPORT SurfacePathDrawer : public QObject
{ Q_OBJECT
public:
    SurfacePathDrawer( ModelViewer*, const std::string& modelUnits="mm");
    virtual ~SurfacePathDrawer(){}

    void setUnits( const std::string&);

    bool isShown() const;
    void show( bool enable);    // Show or hide path information

    void setPathEndPoints( const QPoint&, const QPoint&);

    void setActor( const vtkSmartPointer<vtkActor>);

public slots:
    void doDrawingPath( const QPoint&);
    void doFinishedPath( const QPoint&);

private:
    ModelViewer* _viewer;
    std::string _munits;
    vtkSmartPointer<vtkTextActor> _lenText;
    vtkSmartPointer<vtkCaptionActor2D> _lenCaption;
    QPoint _pathStart;
    RVTK::ModelPathDrawer::Ptr _path;

    SurfacePathDrawer( const SurfacePathDrawer&); // NO COPY
    void operator=( const SurfacePathDrawer&);    // NO COPY
};  // end class

}   // end namespace

#endif


