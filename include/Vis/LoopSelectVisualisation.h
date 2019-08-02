/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_VIS_LOOP_SELECT_VISUALISATION_H
#define FACE_TOOLS_VIS_LOOP_SELECT_VISUALISATION_H

/**
 * Visualisation of a looped region around a centre point.
 */

#include "BaseVisualisation.h"
#include "SphereView.h"
#include "LoopsView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT LoopSelectVisualisation : public BaseVisualisation
{
public:
    explicit LoopSelectVisualisation( double sphereRad=4.0);
    ~LoopSelectVisualisation() override;

    bool applyToAllInViewer() const override { return false;}
    bool applyToSelectedModel() const override { return true;}

    bool belongs( const vtkProp*, const FV*) const override;

    void copy( FV* dst, const FV* src) override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    bool purge( FV*, Action::Event) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    // Set pickability for the sphereView on the associated view (if it exists).
    void setPickable( const FV*, bool);

    // Set whether the visualisation is highlighted or not.
    void setHighlighted( const FV*, bool);

    // Set new loop vertices and reticule position for the given FV.
    void setReticule( const FV*, const cv::Vec3f&);
    void setPoints( const FV*, const std::vector<cv::Vec3f>&);

    void syncActorsToData( const FV*, const cv::Matx44d&) override;

private:
    double _srad;
    std::unordered_map<const FV*, LoopsView*>  _lviews;
    std::unordered_map<const FV*, SphereView*> _sviews;
};  // end class

}}   // end namespaces

#endif
