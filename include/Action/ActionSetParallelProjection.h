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

#ifndef FACE_TOOLS_ACTION_SET_PARALLEL_PROJECTION_H
#define FACE_TOOLS_ACTION_SET_PARALLEL_PROJECTION_H

#include "FaceAction.h"
#include <FaceModelViewer.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionSetParallelProjection : public FaceAction
{ Q_OBJECT
public:
    ActionSetParallelProjection( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Toggle the rendering projection between perspective (normal vision) and orthographic (parallel lines always appear parallel).";}

protected:
    bool checkState( Event) override;
    bool checkEnable( Event) override;
    void doAction( Event) override;
};  // end class

}}   // end namespaces

#endif
