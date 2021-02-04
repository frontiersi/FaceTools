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

#ifndef FACE_TOOLS_VIS_PLANE_VISUALISATION_H
#define FACE_TOOLS_VIS_PLANE_VISUALISATION_H

#include "BaseVisualisation.h"
#include "PlaneView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PlaneVisualisation : public BaseVisualisation
{
public:
    // Choose axis=0 for midsagittal (median) plane,
    //        axis=1 for transverse plane,
    //        axis=2 for frontal (coronal) plane.
    PlaneVisualisation( int axis=0);

    bool applyToAllInViewer() const override { return false;}
    bool applyToAllViewers() const override { return false;}

    bool isAvailable( const FV*) const override;

    void refresh( FV*) override;
    void purge( const FV*) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

    void syncTransform( const FV*) override;

private:
    const int _axis;
    Vec3f _col;
    std::unordered_map<const FV*, PlaneView> _views;
};  // end class

}}   // end namespace

#endif
