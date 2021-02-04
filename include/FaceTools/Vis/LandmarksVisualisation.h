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

#ifndef FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H
#define FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H

#include "BaseVisualisation.h"
#include "LandmarkSetView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LandmarksVisualisation : public BaseVisualisation
{
public:
    bool applyToAllInViewer() const override { return true;}
    bool applyToAllViewers() const override { return true;}
    bool isAvailable( const FV*) const override;
    bool belongs( const vtkProp*, const FV*) const override;

    void refresh( FV*) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncTransform( const FV*) override;

    // Colour the associated landmarks in the selected colour
    // if v true, otherwise set to the unselected colour.
    void setSelectedColour( const FV*, bool v=true);

    // Set the landmarks as being pickable or not.
    void setPickable( const FV*, bool);

    // Turn the given landmark's label for the given FV on/off.
    void setLabelVisible( const FV*, int, FaceSide, bool);
    void setLandmarkHighlighted( const FV*, int, FaceSide, bool);
    void refreshLandmarkPosition( const FV*, int, FaceSide);

    // Return ID of landmark if given prop is for a landmark or -1 if not.
    // On return of >= 0, lat is set to the face side that the landmark appears.
    int landmarkId( const FV*, const vtkProp*, FaceSide& lat) const;

private:
    std::unordered_map<const FV*, LandmarkSetView> _views;
    bool _hasView( const FV*) const;
};  // end class

}}   // end namespaces

#endif
