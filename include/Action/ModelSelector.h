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

#ifndef FACE_TOOLS_MODEL_SELECTOR_H
#define FACE_TOOLS_MODEL_SELECTOR_H

/**
 * Handles FaceControl selections programmatically and via internal interactor.
 */

#include <FaceModelViewer.h>
#include <ModelSelectInteractor.h>

namespace FaceTools {
namespace Action {
class FaceActionManager;

class FaceTools_EXPORT ModelSelector : public QObject
{ Q_OBJECT
public:
    Interactor::ModelSelectInteractor* interactor() { return _interactor;}

    const FaceControlSet& selected() const { return _interactor->selected();}
    const FaceControlSet& available() const { return _interactor->available();}

    // Create a new FaceControl instances and attach it to the given viewer.
    // If given viewer is null , FaceControl added to currently selected viewer.
    FaceControl* addFaceControl( FaceModel*, FaceModelViewer *v=nullptr);

    // Detach the FaceControl from its viewer and delete it.
    void removeFaceControl( FaceControl*);

    // Call removeFaceControl for ALL associated FaceControls of the given FaceModel.
    void remove( FaceModel*);

    // Programmatically select/deselect the given FaceControl.
    void select( FaceControl*, bool);

signals:
    void onSelected( FaceControl*, bool);

private:
    Interactor::ModelSelectInteractor *_interactor;
    explicit ModelSelector( FaceModelViewer* defaultViewer);
    ~ModelSelector() override;
    friend class FaceActionManager;
};  // end class

}   // end namespace
}   // end namespace

#endif
