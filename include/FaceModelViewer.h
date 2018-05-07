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
#include "FaceControlSet.h"

namespace FaceTools {

class FaceModel;

class FaceTools_EXPORT FaceModelViewer : public ModelViewer
{ Q_OBJECT
public:
    explicit FaceModelViewer( QWidget *parent=NULL);

    bool attach( FaceControl*);
    bool detach( FaceControl*);

    const FaceControlSet& attached() const { return _attached;}
    bool isAttached( FaceControl *fc) const { return _attached.has(fc);}

    FaceControl* get( FaceModel* fm) const; // Pointer to view/control of given model or NULL if model not attached.

public slots:
    void resetCamera();
    void saveScreenshot() const;

signals:
    void toggleZeroArea( bool);             // When going from positve to zero viewing area (true) and back (false).

protected:
    void resizeEvent( QResizeEvent*) override;

private:
    FaceControlSet _attached; // All attached FaceControl instances
    std::unordered_map<FaceModel*, FaceControl*> _models;
};  // end class

}   // end namespace

#endif
