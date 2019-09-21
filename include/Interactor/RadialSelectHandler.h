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

#ifndef FACE_TOOLS_RADIAL_SELECT_HANDLER_H
#define FACE_TOOLS_RADIAL_SELECT_HANDLER_H

#include "PropHandler.h"
#include "../Vis/RadialSelectVisualisation.h"
#include <ObjModelRegionSelector.h> // RFeatures

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT RadialSelectHandler : public PropHandler
{ Q_OBJECT
public:
    RadialSelectHandler( Vis::RadialSelectVisualisation&, const FM*);

    // Return the model this region selector is for.
    const FM* model() const { return _model;}

    // Use after initialisation (centre and radius managed internally thereafter).
    void set( const cv::Vec3f& centre, double radius);

    double radius() const;
    cv::Vec3f centre() const;
    size_t selectedFaces( IntSet&) const;

private slots:
    void doEnterProp( Vis::FV*, const vtkProp*) override;
    void doLeaveProp( Vis::FV*, const vtkProp*) override;

private:
    bool leftButtonDown() override;
    bool leftButtonUp() override;
    bool leftDrag() override;

    bool mouseWheelForward() override;
    bool mouseWheelBackward() override;

    Vis::RadialSelectVisualisation &_vis;
    bool _onReticule, _move;
    const FM* _model;   // The model the region selector is for.
    double _radiusChange;
    RFeatures::ObjModelRegionSelector::Ptr _rsel;
    void _showHover(bool);
};  // end class

}}   // end namespaces

#endif
