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

#ifndef FACE_TOOLS_ACTION_MAP_CURVATURE_H
#define FACE_TOOLS_ACTION_MAP_CURVATURE_H

#include "FaceAction.h"
#include <QStatusBar>
#include <ObjModelCurvatureMetrics.h>   // RFeatures

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionMapCurvature : public FaceAction
{ Q_OBJECT
public:
    explicit ActionMapCurvature( QProgressBar* pb=NULL);    // Async if pb not NULL
    ~ActionMapCurvature() override;

    QString getDisplayName() const override { return "Compute Curvature";}
    const QIcon* getIcon() const override { return &_icon;}

    // Returns the metrics for the given FaceControl. If not already cached, the data
    // are computed but only if testReady(fc) returns true for given FaceControl fc.
    RFeatures::ObjModelCurvatureMetrics::Ptr metrics( FaceControl*);

protected slots:
    bool testReady( FaceControl*) override;
    bool doAction( FaceControlSet&) override;
    void respondToChange( FaceControl*) override;
    void burn( const FaceControl*) override;

private:
    QIcon _icon;
    std::unordered_map<const FaceModel*, RFeatures::ObjModelCurvatureMetrics::Ptr> _cmaps;
};  // end class

}   // end namespace
}   // end namespace

#endif
