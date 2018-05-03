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

#ifndef FACE_TOOLS_MODEL_VIEWER_INTERACTOR_H
#define FACE_TOOLS_MODEL_VIEWER_INTERACTOR_H

/**
 * Provides an interface to a model viewer via VTK events on the underlying viewer.
 * Multiple ModelViewerInteractor instances can be active on a single viewer at once,
 * but a single ModelViewerInteractor can only be attached to one viewer.
 * Within its constructor, this interface attaches itself to a viewer using
 * ModelViewer::attachInterface( &VVI) and detaches itself on destruction with
 * ModelViewer::detachInterface( &VVI).
 */

#include "FaceTools_Export.h"
#include <VtkViewerInteractor.h>    // QTools::VVI
#include <QPoint>

namespace FaceTools {
class ModelViewer;

namespace Interactor {

class FaceTools_EXPORT ModelViewerInteractor : public QTools::VVI
{
public:
    explicit ModelViewerInteractor( ModelViewer*);
    virtual ~ModelViewerInteractor();

    const ModelViewer* viewer() { return _viewer;}      // Get the attached viewer (or NULL)

protected:
    // Lock/unlock camera/actor interaction.
    void setInteractionLocked( bool v);
    bool isInteractionLocked() const;

private:
    ModelViewer *_viewer;
};  // end class

typedef ModelViewerInteractor MVI;

}   // end namespace
}   // end namespace

#endif
