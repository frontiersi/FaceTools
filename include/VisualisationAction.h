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
#include <boost/unordered_set.hpp>
#include <vtkSmartPointer.h>
#include <vtkActor.h>


namespace FaceTools
{

// The interface to interact with discovered visualisation plugins
class FaceTools_EXPORT VisualisationAction : public FaceAction
{ Q_OBJECT
public:
    VisualisationAction();

    // If this visualisation is available for the given model.
    virtual bool isAvailable( const FaceControl*) const { return false;}

    // How visualisation options apply
    virtual bool useTexture() const { return false;}     // Use texture mapping?
    virtual bool allowSetVertexSize() const { return false;}
    virtual bool allowSetWireframeLineWidth() const { return false;}
    virtual bool allowSetColour() const { return false;}
    virtual bool allowScalarVisualisation( float& minv, float& maxv) const { return false;}

    virtual void mapActor( FaceControl*) = 0;

    // Default visualisations for a FaceControl when added.
    void addController( FaceControl*) override;
    void removeController( FaceControl*) override;
    void setControlled( FaceControl*, bool) override;

protected:
    bool doAction() override;    // Apply this visualisation over all views.

    // If this is the default visualistion for a model upon first adding.
    virtual bool isDefault( const FaceControl*) const { return false;}

private:
    boost::unordered_set<FaceControl*> _fconts;
    void recheckCanVisualise( FaceControl*);
};  // end class

}   // end namespace

#endif

