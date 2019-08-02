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

#ifndef FACE_TOOLS_VIS_SURFACE_VISUALISATION_H
#define FACE_TOOLS_VIS_SURFACE_VISUALISATION_H

/**
 * Abstract application of scalars or vectors to an actor's surface polygons.
 */

#include "BaseVisualisation.h"
#include "SurfaceMetricsMapper.h"
#include <FaceViewSet.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT SurfaceVisualisation : public BaseVisualisation
{
public:
    explicit SurfaceVisualisation( SurfaceMetricsMapper::Ptr);

    bool isExclusive() const override { return true;}

    // Returns true iff this visualisation can be used to map the given data.
    bool isAvailable( const FM*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    bool purge( FV*, Action::Event) override;

    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;

private:
    SurfaceMetricsMapper::Ptr _smm;
    FVS _mapped;
};  // end class

}}   // end namespace

#endif
