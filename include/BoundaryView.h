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

#ifndef FACE_TOOLS_BOUNDARY_VIEW_H
#define FACE_TOOLS_BOUNDARY_VIEW_H

#include "ModelViewer.h"
#include "ObjMetaData.h"
#include "VisualisationOptions.h"

namespace FaceTools
{

class FaceTools_EXPORT BoundaryView : public QObject
{ Q_OBJECT
public:
    BoundaryView( ModelViewer*, const ObjMetaData::Ptr);

    void show( bool enable);
    bool isShown() const;
    void reset();
    void setVisualisationOptions( const VisualisationOptions::Boundary&);

public slots:
    void setFaceCropFactor( double);

private:
    ModelViewer *_viewer;
    const ObjMetaData::Ptr _omd;
    double _faceCropFactor;
    bool _isshown;
    VisualisationOptions::Boundary _visopts;
    vtkSmartPointer<vtkActor> _boundary;

    BoundaryView( const BoundaryView&);   // No copy
    void operator=( const BoundaryView&); // No copy
};  // end class

}   // end namespace

#endif
