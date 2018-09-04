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

#ifndef FACE_TOOLS_MODEL_SELECTOR_H
#define FACE_TOOLS_MODEL_SELECTOR_H

/**
 * Handles FaceView selections programmatically and via internal interactor.
 */

#include <FaceModelViewer.h>
#include <ModelSelectInteractor.h>

namespace FaceTools {
namespace Action {
class FaceActionManager;

class FaceTools_EXPORT ModelSelector : public QObject
{ Q_OBJECT
public:
    Interactor::ModelSelectInteractor* interactor() { return &_interactor;}

    Vis::FV* selected() const { return _interactor.selected();}
    const FVS& available() const { return _interactor.available();}

    // Create a new FaceView instances and attach it to the given viewer.
    // If given viewer is null, FaceView added to currently selected viewer.
    // Returned FaceView is NOT automatically selected.
    Vis::FV* addFaceView( FM*, FaceModelViewer *v=nullptr);

    // Detach the FaceView from its viewer and delete it.
    void removeFaceView( Vis::FV*);

    // Call removeFaceView for ALL associated FaceViews of the given FaceModel.
    void remove( FM*);

    // Programmatically select/deselect the given FaceView.
    void setSelected( Vis::FV*, bool);

signals:
    void onSelected( Vis::FV*, bool);

private slots:
    // Set the selected FaceView to be the one on the given viewer having
    // the same FaceModel as the currently selected FaceView. Does nothing
    // if there's no FaceView on the given viewer with the same FaceModel as
    // the currently selected (or if there's no currently selected FaceView).
    void doSwitchSelectedToViewer( ModelViewer*);

private:
    Interactor::ModelSelectInteractor _interactor;
    explicit ModelSelector( FaceModelViewer* defaultViewer);
    ~ModelSelector() override;
    friend class FaceActionManager;
};  // end class

}   // end namespace
}   // end namespace

#endif
