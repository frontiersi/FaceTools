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

#ifndef FACE_TOOLS_RADIAL_SELECT_HANDLER_H
#define FACE_TOOLS_RADIAL_SELECT_HANDLER_H

#include "GizmoHandler.h"
#include <FaceTools/Vis/RadialSelectVisualisation.h>
#include <r3d/RegionSelector.h>
#include <r3d/Boundaries.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT RadialSelectHandler : public GizmoHandler
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<RadialSelectHandler>;
    static Ptr create();

    void postRegister() override;
    void refresh() override;

    Vis::BaseVisualisation &visualisation() { return _vis;}
    const Vis::BaseVisualisation &visualisation() const { return _vis;}

    // Initialise the handler to work with the given view's model.
    void init( const FM*, const Vec3f& centre, float radius);

    float radius() const;
    Vec3f centre() const;   // Returns transformed point

    // Size of selected face ID set will always be >= 1 if initialised.
    const IntSet& selectedFaces() const { return _fids;}
    const std::list<int> &boundaryVertices() const;

private:
    bool doEnterProp() override;
    bool doLeaveProp() override;

    bool doLeftButtonDown() override;
    bool doLeftButtonUp() override;
    bool doLeftDrag() override;

    bool doMouseWheelForward() override;
    bool doMouseWheelBackward() override;

    Vis::RadialSelectVisualisation _vis;
    bool _onReticule;
    bool _moving;
    float _radiusChange;
    const Vis::BaseVisualisation *_lmkVis;
    IntSet _fids;           // The currently selected facets
    r3d::Boundaries _bnds;  // And corresponding boundary vertices
    r3d::RegionSelector::Ptr _rsel;

    bool _isRegionSelectorForModel( const FM*) const;
    void _update( Vec3f, float);
    void _showHover();
    bool _changeRadius( float);
    bool _testInProp( bool);

    RadialSelectHandler();
};  // end class

}}   // end namespaces

#endif
