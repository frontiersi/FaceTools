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

#ifndef FACE_TOOLS_ACTION_MAP_SURFACE_DATA_H
#define FACE_TOOLS_ACTION_MAP_SURFACE_DATA_H

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionMapSurfaceData : public FaceAction
{ Q_OBJECT
public:
    ActionMapSurfaceData();

protected slots:
    bool doAction( FVS&, const QPoint&) override;
    void purge( const FaceModel*) override;
    void doOnCalculated( const FaceModel*);
    bool displayDebugStatusProgression() const override { return false;}
};  // end class

}   // end namespace
}   // end namespace

#endif
