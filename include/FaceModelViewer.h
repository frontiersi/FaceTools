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

#ifndef FACE_TOOLS_FACE_MODEL_VIEWER_H
#define FACE_TOOLS_FACE_MODEL_VIEWER_H

/*
 * FaceModelViewer that allows for model(s) to be selected with duplicate
 * views (FaceView/FaceControl) of the same model (FaceModel) disallowed.
 */

#include "ModelViewer.h"
#include <ModelSelectInteractor.h>
#include <unordered_map>

namespace FaceTools {

class FaceModel;

class FaceTools_EXPORT FaceModelViewer : public QWidget, public ModelViewer
{ Q_OBJECT
public:
    FaceModelViewer( QWidget *parent=NULL, bool exclusiveSelect=true);  // Exclusive (single) model selection by default
    ~FaceModelViewer() override;

    // Only models that are currently attached to this viewer may be (de)selected.
    bool attach( FaceControl*);
    bool detach( FaceControl*);
    const FaceControlSet& attached() const; // The currently attached set.
    bool isAttached( FaceControl*) const;   // Returns true iff model is attached to this viewer.

    size_t count() const { return _attached.size();}  // Returns the count of the number of *attached* models in this viewer.

    FaceControl* get( FaceModel* fm) const; // Return pointer to view/control of given model or NULL if model not attached.

    const FaceControlSet& selected() const; // The currently selected models (at most one member if exclusiveSelect).
    void setSelected( FaceControl*, bool);  // Set the given FaceControl as selected (true) or deselected (false)
    bool isSelected( FaceControl*) const;   // Returns true iff model is currently selected.

public slots:
    void resetCamera();
    void saveScreenshot() const;

signals:
    void toggleZeroArea( bool);                 // When going from positve to zero viewing area (true) and back (false).
    void onUserSelected( FaceControl*, bool);   // Fires when user changes selection 

protected:
    void resizeEvent( QResizeEvent*) override;

private:
    std::unordered_map<FaceModel*, FaceControl*> _attached;
    Interactor::ModelSelectInteractor* _selector;
};  // end class

}   // end namespace

#endif
