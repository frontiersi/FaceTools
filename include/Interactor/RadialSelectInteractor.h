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

#ifndef FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H
#define FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H

#include "ModelEntryExitInteractor.h"
#include <LoopSelectVisualisation.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT RadialSelectInteractor : public ModelViewerInteractor 
{ Q_OBJECT
public:
    RadialSelectInteractor( MEEI*, Vis::LoopSelectVisualisation*, QStatusBar* sbar=nullptr);

signals:
    void onIncreaseRadius( const Vis::FV*);
    void onDecreaseRadius( const Vis::FV*);
    void onSetCentre( const Vis::FV*, const cv::Vec3f&);

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool leftDrag( const QPoint&) override;
    bool mouseMove( const QPoint&) override;
    bool mouseWheelForward( const QPoint&) override;
    bool mouseWheelBackward( const QPoint&) override;
    void onEnabledStateChanged(bool) override;
    void doOnEnterReticule();
    void doOnLeaveReticule();

    MEEI *_meei;
    Vis::LoopSelectVisualisation *_vis;
    bool _move;
    bool _onReticule;
    Vis::FV* _model;
    static const QString s_msg;
};  // end class

}   // end namespace
}   // end namespace

#endif
