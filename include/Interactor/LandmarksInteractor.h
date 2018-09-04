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
    inline int hoverId() const { return _hover;}
    inline Vis::FV* hoverModel() const { return _meei->model();}

private slots:
    void doOnEnterLandmark( const Vis::FV*, int);
    void doOnLeaveLandmark( const Vis::FV*, int);

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool leftDrag( const QPoint&) override;
    void onEnabledStateChanged( bool v);

    MEEI *_meei;
    Vis::LandmarksVisualisation *_vis;
    int _drag, _hover;  // IDs of the landmarks being dragged and hovered over.
    Vis::FV* _view;
    static const QString s_defaultMsg;
    static const QString s_moveMsg;
};  // end class

}   // end namespace
}   // end namespace

#endif
