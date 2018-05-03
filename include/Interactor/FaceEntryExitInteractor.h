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

#ifndef FACE_TOOLS_FACE_ENTRY_EXIT_INTERACTOR_H
#define FACE_TOOLS_FACE_ENTRY_EXIT_INTERACTOR_H

/**
 * Deals with messages relating to position of the mouse cursor as it moves over a face model.
 * Handy for actions needing to enable/disable themselves depending upon where the mouse is.
 */

#include "ModelViewerInteractor.h"
#include <FaceModelViewer.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT FaceEntryExitInteractor : public QObject, public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit FaceEntryExitInteractor( FaceModelViewer*);

    // Model the cursor is currently over (NULL if none).
    inline FaceControl* currentModel() const { return _mnow;}

    // Landmark the cursor is currently over (-1 if none).
    inline int currentLandmark() const { return _lnow;}

signals:
    void onEnterModel( FaceControl*);
    void onLeaveModel( FaceControl*);

    // Enter and leave signals for landmarks will only be emitted if the given
    // FaceControl currently has a LandmarksVisualisation applied to its view.
    void onEnterLandmark( FaceControl*, int lmkId);
    void onLeaveLandmark( FaceControl*, int lmkId);

private:
    const FaceModelViewer* _viewer;
    FaceControl* _mnow;
    int _lnow;

    void mouseMove( const QPoint&) override;
    void middleDrag( const QPoint&) override;
    void rightDrag( const QPoint&) override;
    void leftDrag( const QPoint&) override;
    void testPoint( const QPoint&);
};  // end class

}   // end namespace
}   // end namespace

#endif
