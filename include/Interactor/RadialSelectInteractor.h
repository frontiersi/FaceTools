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

/**
 * Provides for user definition of a radial region on a model. Uses FaceEntryExitInteractor
 * so can only be attached to FaceModelViewer and not the base ModelViewer type.
 */

#include "ModelViewerInteractor.h"
#include <BoundingVisualisation.h>

namespace FaceTools {
namespace Interactor {
class FaceEntryExitInteractor;

class FaceTools_EXPORT RadialSelectInteractor : public ModelViewerInteractor 
{ Q_OBJECT
public:
    explicit RadialSelectInteractor( Vis::BoundingVisualisation*);
    ~RadialSelectInteractor() override;

signals:
    // Signals that notify client of the parameters needed to update
    // the BoundingVisualisation for the given FaceControl.
    void onSetNewCentre( FaceControl*, const cv::Vec3f&);
    void onSetNewRadius( FaceControl*, double);

protected:
    void onAttached() override;
    void onDetached() override;

private:
    bool leftDoubleClick( const QPoint&) override;
    bool leftDrag( const QPoint&) override;
    bool leftButtonUp( const QPoint&) override;
    bool mouseWheelForward( const QPoint&) override;
    bool mouseWheelBackward( const QPoint&) override;

    Vis::BoundingVisualisation *_bvis;
    FaceEntryExitInteractor *_feei;
    FaceControl *_fc;
    bool _move;
};  // end class

}   // end namespace
}   // end namespace

#endif
