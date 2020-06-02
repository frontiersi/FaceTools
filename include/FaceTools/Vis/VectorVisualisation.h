/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_VIS_VECTOR_VISUALISATION_H
#define FACE_TOOLS_VIS_VECTOR_VISUALISATION_H

/**
 * Represents a single 'kind' of vector visualisation (e.g. normals). The data for
 * each view instance is held within this object and new data is asked for only when
 * needed. The decision to purge is deferred to the derived type, and new mapping
 * data (float array) is asked for from the derived type if none exists for the
 * passed in view.
 */

#include "ScalarVisualisation.h"
#include <FaceTools/FaceModel.h>
#include <r3dvis/VtkVectorMap.h>
#include <vtkFloatArray.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT VectorVisualisation : public SurfaceVisualisation<VectorVisualisation> // static polymorphism
{
public:
    using Ptr = std::shared_ptr<VectorVisualisation>;

    // label:   name of the data range mapped to actors,
    // mapp:    true if this mapper maps polygons; false if maps vertices
    VectorVisualisation( const std::string& label, bool mapp);
    virtual ~VectorVisualisation(){}

    const char* name() const override { return "VectorVisualisation";}

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    // Use the provided scalar mapper's lookup table to colour the vectors.
    void setScalarMapping( const FV*, ScalarVisualisation*);

    // Called from FaceView::setActiveVectors (FaceView tracks which VectorVisualisation
    // is currently active on it so it knows to keep only one active at any one time).
    void activate( FV*);
    void deactivate( FV*);

    // Must be overridden to return true if visualisation mapping available for model.
    virtual bool isAvailable( const FV*) const { return true;}

    void syncWithViewTransform( const FV*) override;

private:
    std::unordered_map<const FV*, r3dvis::VtkVectorMap::Ptr> _glyphs;
};  // end class

}}   // end namespaces

#endif
