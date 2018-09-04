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

#ifndef FACE_TOOLS_MODEL_VIEWER_ENTRY_EXIT_INTERACTOR_H
#define FACE_TOOLS_MODEL_VIEWER_ENTRY_EXIT_INTERACTOR_H

/**
 * Signals onLeave and onEnter when leaving and entering respectively the attached ModelViewer.
 * Used by ViewerInteractionManager to ensure that interactors are always set on the viewer
 * underneath the mouse cursor.
 */

#include "ModelViewerInteractor.h"

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ModelViewerEntryExitInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit ModelViewerEntryExitInteractor( ModelViewer *v)
        : ModelViewerInteractor(v) {}

signals:
    void onLeave( const QPoint&);
    void onEnter( const QPoint&);

private:
    bool mouseLeave( const QPoint& p) override { emit onLeave(p); return false;}
    bool mouseEnter( const QPoint& p) override { emit onEnter(p); return false;}
};  // end class

}   // end namespace
}   // end namespace

#endif
