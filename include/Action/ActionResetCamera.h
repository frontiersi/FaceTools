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

#ifndef FACE_TOOLS_ACTION_RESET_CAMERA_H
#define FACE_TOOLS_ACTION_RESET_CAMERA_H

#include "FaceAction.h"
#include <FaceModelViewer.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionResetCamera : public FaceAction
{ Q_OBJECT
public:
    ActionResetCamera( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Reset the camera in the viewer of the currently selected model to focus on it, or reset all viewer cameras to their default position and focus if no model is selected.";}

    static void resetCamera( const Vis::FV*);

protected:
    void doAction( Event) override;
};  // end class

}}   // end namespace

#endif
