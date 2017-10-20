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

#ifndef FACE_TOOLS_SURFACE_VISUALISATION_H
#define FACE_TOOLS_SURFACE_VISUALISATION_H

#include "VisualisationAction.h"

namespace FaceTools {

class FaceTools_EXPORT SurfaceVisualisation : public VisualisationAction
{ Q_OBJECT
public:
    SurfaceVisualisation( const std::string& iconfile="", bool isDefaultVis=false);
    virtual ~SurfaceVisualisation(){}

    virtual const QIcon* getIcon() const { return &_icon;}
    virtual QString getDisplayName() const { return "Surface";}

    virtual bool isAvailable( const FaceModel*) const { return true;}
    virtual vtkSmartPointer<vtkActor> makeActor( const FaceModel*);
    virtual bool useFloodLights() const { return false;}
    virtual bool allowSetBackfaceCulling() const { return true;}
    virtual bool allowSetVertexSize() const { return false;}
    virtual bool allowSetWireframeLineWidth() const { return false;}
    virtual bool allowSetColour() const { return true;}
    virtual bool allowScalarVisualisation( float& minv, float& maxv) const { return false;}

private:
    const QIcon _icon;
};  // end class

}   // end namespace

#endif


