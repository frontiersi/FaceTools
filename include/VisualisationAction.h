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

#ifndef FACE_TOOLS_VISUALISATION_ACTION_H
#define FACE_TOOLS_VISUALISATION_ACTION_H

#include "FaceActionInterface.h"
#include <vtkSmartPointer.h>
#include <vtkActor.h>

namespace FaceTools
{

class FaceView;
class FaceModel;

// The interface to interact with discovered visualisation plugins
class FaceTools_EXPORT VisualisationAction : public FaceAction
{ Q_OBJECT
public:
    VisualisationAction();
    virtual ~VisualisationAction(){}

    virtual void connectInteractor( ModelInteractor*);
    virtual void disconnectInteractors();

    // If this visualisation is currently available.
    virtual bool isAvailable( const FaceModel*) const = 0;

    // Make the actor for the given model.
    virtual vtkSmartPointer<vtkActor> makeActor( const FaceModel*) = 0;

    // How visualisation options apply
    virtual bool useFloodLights() const = 0; // Use headlight if return false.
    virtual bool allowSetBackfaceCulling() const = 0;
    virtual bool allowSetVertexSize() const = 0;
    virtual bool allowSetWireframeLineWidth() const = 0;
    virtual bool allowSetColour() const = 0;
    virtual bool allowScalarVisualisation( float& minv, float& maxv) const = 0;

protected:
    virtual bool doAction();

private slots:
    void checkAllow(bool);
    void doOnMeshUpdated();

private:
    FaceView *_fview;
    const FaceModel *_fmodel;
};  // end class

}   // end namespace

#endif

