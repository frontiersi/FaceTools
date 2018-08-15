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

#ifndef FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H
#define FACE_TOOLS_RADIAL_SELECT_INTERACTOR_H

#include "FaceEntryExitInteractor.h"
#include <RadialSelectVisualisation.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT RadialSelectInteractor : public ModelViewerInteractor 
{ Q_OBJECT
public:
    RadialSelectInteractor( FEEI*, Vis::RadialSelectVisualisation*, QStatusBar* sbar=nullptr);

protected:
    void onEnabledStateChanged(bool) override;

private:
    bool leftButtonDown( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool leftDrag( const QPoint&) override;
    bool mouseMove( const QPoint&) override;
    bool mouseWheelForward( const QPoint&) override;
    bool mouseWheelBackward( const QPoint&) override;
    bool testOnReticule( const QPoint&) const;

    FEEI *_feei;
    Vis::RadialSelectVisualisation *_vis;
    bool _move;
    FaceControl* _model;
    static const QString s_msg;
};  // end class

}   // end namespace
}   // end namespace

#endif
