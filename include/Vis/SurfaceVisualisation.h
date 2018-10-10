/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include "SurfaceDataMapper.h"
#include <FaceViewSet.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT SurfaceVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    SurfaceVisualisation( SurfaceDataMapper::Ptr, const QIcon& ico=QIcon());

    bool isToggled() const override { return false;}    // Non-toggle so this visualisation is exclusive.

    // Returns true iff this visualisation can be used to map the given data.
    bool isAvailable( const FM* fm) const override { return _smapper->isAvailable(fm);}

    void apply( FV*, const QPoint* mc=nullptr) override;
    void clear( FV*) override;

    void purge( FV*) override;
    void purge( const FM*) override;
    bool allowShowOnLoad( const FM*) const override { return false;}

private:
    SurfaceDataMapper::Ptr _smapper;
    FVS _mapped;
};  // end class

}   // end namespace
}   // end namespace

#endif
