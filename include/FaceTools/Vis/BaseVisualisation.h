/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_VIS_BASE_VISUALISATION_H
#define FACE_TOOLS_VIS_BASE_VISUALISATION_H

#include "FaceView.h"

namespace FaceTools { namespace Vis {

class BaseVisualisation
{
public:
    BaseVisualisation(){}
    virtual ~BaseVisualisation(){}

    // By default, visualisation is applied just to the selected view. The visualisation is
    // applied to all views in the selected view's viewer if applyToAllInViewer is overridden
    // to return true. Overriding applyToAllViewers to return true will cause the visualisation
    // to be applied to all views of the selected model.
    virtual bool applyToAllInViewer() const { return false;}
    virtual bool applyToAllViewers() const { return false;}

    // Return true iff given prop relating to the given FV belongs to this visualisation.
    // Generally only need to override if the visualisation is used in interactions.
    virtual bool belongs( const vtkProp*, const FV*) const { return false;}

    // Returns true iff the given FaceView can be refreshed with visualisation data.
    virtual bool isAvailable( const FV*) const { return true;}

    // Set the visibility of applied visualisations.
    virtual bool isVisible( const FV*) const { return false;}
    virtual void setVisible( FV*, bool) {}

    // Specify the min and max allowed opacity of the face when this visualisation is applied.
    // If multiple visualisations are applied, the min/max settings are chosen to be the
    // highest min and the lowest max over all visible visualisations.
    virtual float minAllowedOpacity() const { return 0.0f;}
    virtual float maxAllowedOpacity() const { return 1.0f;}

    // Refresh availability of the visualisation arrays/actors.
    virtual void refresh( FV*) {}

    // Cause visualisation to update any actor transforms to match the FaceView actor transform.
    // This may not be the same as the FaceModel transform since the FaceView actor can be moved
    // interactively with the FaceModel transform only being updated after interaction finishes.
    virtual void syncTransform( const FV*){}

    // Destroy any created actors.
    virtual void purge( const FV*) {}

private:
    BaseVisualisation( const BaseVisualisation&) = delete;
    void operator=( const BaseVisualisation&) = delete;
};  // end class

}}   // end namespaces

#endif
