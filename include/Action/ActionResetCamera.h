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

#ifndef FACE_TOOLS_ACTION_RESET_CAMERA_H
#define FACE_TOOLS_ACTION_RESET_CAMERA_H

#include "FaceAction.h"
#include <FaceModelViewer.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionResetCamera : public FaceAction
{ Q_OBJECT
public:
    // If no viewers are added, viewers that are reset will be taken from the FVS passed in to doAction.
    ActionResetCamera( const QString& dname="Reset Camera", const QIcon& ico=QIcon(), FaceModelViewer *v=nullptr);

    void addViewer( FaceModelViewer* v) { _viewers.push_back(v);}

private slots:
    bool testEnabled( const QPoint* mc=nullptr) const override { return true;}
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(CAMERA_CHANGE);}
    bool displayDebugStatusProgression() const override { return false;}

private:
    std::vector<FaceModelViewer*> _viewers;
};  // end class

}   // end namespace
}   // end namespace

#endif
