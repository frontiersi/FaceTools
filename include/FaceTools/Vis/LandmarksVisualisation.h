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

#ifndef FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H
#define FACE_TOOLS_VIS_LANDMARKS_VISUALISATION_H

#include "BaseVisualisation.h"
#include "LandmarkSetView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LandmarksVisualisation : public BaseVisualisation
{
public:
    LandmarksVisualisation();
    ~LandmarksVisualisation() override;

    const char* name() const override { return "LandmarksVisualisation";}

    bool applyToAllInViewer() const override { return false;}
    bool applyToAllViewers() const override { return false;}
    bool isAvailable( const FV*, const QPoint*) const override;
    bool belongs( const vtkProp*, const FV*) const override;

    void apply( const FV*, const QPoint* mc=nullptr) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void refreshState( const FV*) override; // Update all landmark view positions from data
    void syncWithViewTransform( const FV*) override;

    // Turn the given landmark's label for the given FV on/off.
    void setLabelVisible( const FV*, int, FaceSide, bool);
    void setLandmarkHighlighted( const FV*, int, FaceSide, bool);
    void refreshLandmark( const FV*, int);  // Refresh position of the given landmark.

    // Return ID of landmark if given prop is for a landmark or -1 if not.
    // On return of >= 0, lat is set to the face side that the landmark appears.
    int landmarkId( const FV*, const vtkProp*, FaceSide& lat) const;

private:
    std::unordered_map<const FV*, LandmarkSetView*> _views;
    const Vis::FV *_oldfv;
    bool _hasView( const FV*) const;
};  // end class

}}   // end namespaces

#endif
