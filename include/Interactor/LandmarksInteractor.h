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

#ifndef FACE_TOOLS_LANDMARKS_INTERACTOR_H
#define FACE_TOOLS_LANDMARKS_INTERACTOR_H

#include "ModelEntryExitInteractor.h"
#include <LandmarksVisualisation.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT LandmarksInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    LandmarksInteractor( MEEI*, Vis::LandmarksVisualisation*, QStatusBar* sbar=nullptr);

    // Get the ID of the landmark being hovered over (if any).
    // Returns -1 if no handle hovered over. Use hoverModel() to get associated model.
    int hoverID() const { return _hover;}
    FaceControl* hoverModel() const { return _meei->model();}

    // Add a landmark on the hover model at the given coords with the given label.
    // Client should check for existing landmarks first before deciding to create a new one.
    // Return id of the newly added landmark or -1 if no landmark could be created
    // (e.g. if the given coords are not on the model).
    int addLandmark( const std::string&, const QPoint&);

    // Deletes the landmark returned by hoverID returning true on success.
    bool deleteLandmark();

protected:
    void onEnabledStateChanged( bool v);

private slots:
    void doOnEnterLandmark( const FaceControl*, int);
    void doOnLeaveLandmark( const FaceControl*, int);

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool leftDrag( const QPoint&) override;

    MEEI *_meei;
    Vis::LandmarksVisualisation *_vis;
    int _drag, _hover;  // IDs of the landmarks being dragged and hovered over.
    FaceControl* _model;
    static const QString s_defaultMsg;
    static const QString s_moveMsg;
};  // end class

}   // end namespace
}   // end namespace

#endif
