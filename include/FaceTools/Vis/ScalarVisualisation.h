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

#ifndef FACE_TOOLS_VIS_SCALAR_VISUALISATION_H
#define FACE_TOOLS_VIS_SCALAR_VISUALISATION_H

/**
 * Represents a single 'kind' of scalar visualisation (e.g. curvature). The data for
 * each view instance is held within this object and new data is asked for only when
 * needed. The decision to purge is deferred to the derived type, and new mapping
 * data (float array) is asked for from the derived type if none exists for the
 * passed in view.
 */

#include "SurfaceVisualisation.h"
#include <QTools/ScalarColourRangeMapper.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT ScalarVisualisation : public SurfaceVisualisation<ScalarVisualisation> // static polymorphism
{
public:
    using Ptr = std::shared_ptr<ScalarVisualisation>;

    // label:   name of the data range mapped to actors,
    // mapp:    true if this mapper maps polygons; false if maps vertices
    // minv:    the minimum metric magnitude allowed
    // maxv:    the maximum metric magnitute allowed
    // stepSize: the value step size - if left as zero, computed as 20 divisions in maxv - minv
    // numStepSize: the number of colour bands to step through at once
    ScalarVisualisation( const std::string& label, bool mapp, float minv, float maxv, float stepSize=0.0f, size_t numStepSize=1);
    virtual ~ScalarVisualisation();

    const char* name() const override { return "ScalarVisualisation";}

    float minRange() const { return _minr;} // Min range initially same as min visible
    float maxRange() const { return _maxr;} // Max range initially same as max visible
    float minVisible() const { return _minv;}
    float maxVisible() const { return _maxv;}
    float stepSize() const { return _ssize;}

    void setNumStepSize( size_t v) { _nssize = v;}
    size_t numStepSize() const { return _nssize;}

    void setVisibleRange( float,float);    // Scalar visible values

    // Return the number of colours used to map the surface of actors (never less than 1).
    size_t numColours() const;
    void setNumColours( size_t);

    QColor minColour() const;
    QColor maxColour() const;
    void setMinColour( const QColor&);
    void setMaxColour( const QColor&);

    vtkLookupTable* lookupTable( const vtkRenderer*);

    void rebuild();

    bool isToggled() const override { return false;}

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    // Called from FaceView::setActiveScalars (FaceView tracks which ScalarVisualisation
    // is currently active on it so it knows to keep only one active at any one time).
    void activate( FV*);
    void deactivate( FV*);

    // Must be overridden to return true if visualisation mapping available for model.
    virtual bool isAvailable( const FV*) const { return true;}

private:
    const float _minr;
    const float _maxr;
    float _minv, _maxv;
    float _ssize;
    size_t _nssize;
    QTools::ScalarColourRangeMapper _cmapper;
    FVS _activated;
};  // end class

}}   // end namespaces

#endif
