/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <ObjModelCurvatureMetrics.h>   // RFeatures

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionMapSurfaceData : public FaceAction
{ Q_OBJECT
public:
    explicit ActionMapSurfaceData( const QString& dname);  // Asynchronous
    ~ActionMapSurfaceData() override;

    // Returns surface data for the given FaceControl (data must already be present!).
    const RFeatures::ObjModelCurvatureMetrics* metrics( const FaceControl*);
    const RFeatures::ObjModelCurvatureMap* curvature( const FaceControl*);
    const RFeatures::ObjModelNormals* normals( const FaceControl*);

protected slots:
    bool testReady( FaceControl*) override;
    bool doAction( FaceControlSet&) override;
    void respondTo( const FaceAction*, const ChangeEventSet*, FaceControl*) override;
    void purge( const FaceControl*) override;

private:
    struct SurfaceData;
    std::unordered_map<const FaceModel*, SurfaceData*> _cmaps;
};  // end class

}   // end namespace
}   // end namespace

#endif
