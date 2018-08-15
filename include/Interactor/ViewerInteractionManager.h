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

#ifndef FACE_TOOLS_VIEWER_INTERACTION_MANAGER_H
#define FACE_TOOLS_VIEWER_INTERACTION_MANAGER_H

#include <ModelViewerEntryExitInteractor.h>
#include <unordered_map>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ViewerInteractionManager : public QObject
{ Q_OBJECT
public:
    // Set with the initial viewer that all interactors will initially be attached to.
    // Extra viewers that the interactors will be moved between should be added using
    // function addViewer.
    explicit ViewerInteractionManager( ModelViewer*);
    ~ViewerInteractionManager() override;

    void addViewer( ModelViewer*);

signals:
    // Emitted after the mouse has entered the given viewer.
    // All interactors will be active on the given viewer.
    // All other viewers won't be receiving user input events.
    void onActivatedViewer( ModelViewer*);

private slots:
    void doOnViewerEntered();

private:
    ModelViewer *_activeViewer;
    std::unordered_map<ModelViewerEntryExitInteractor*, ModelViewer*> _vmap;

    ViewerInteractionManager( const ViewerInteractionManager&) = delete;
    void operator=( const ViewerInteractionManager&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
