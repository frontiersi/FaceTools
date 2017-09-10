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
#include <boost/unordered_set.hpp>

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

    // If this visualisation is available for the given model.
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

    virtual void removeInteractor( ModelInteractor*);
    virtual void setInteractive( ModelInteractor*, bool);

protected:
    virtual bool doAction();

private slots:
    void doOnMeshUpdated();

private:
    boost::unordered_set<ModelInteractor*> _interactors;
};  // end class

}   // end namespace

#endif

