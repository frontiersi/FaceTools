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

// TODO
/**
 * Provides for user definition of a radial region of a model.
 * Upon attaching to a viewer, interactor locks the camera to
 * ensure that normal camera navigation actions are ignored to
 * allow this interface to act on the viewer's contents.
 */

#include "ModelViewerInteractor.h"

namespace FaceTools {
namespace Interactor {

class FaceModelViewer;

class FaceTools_EXPORT RadialSelectInteractor : public ModelViewerInteractor 
{
public:
    explicit RadialSelectInteractor( FaceModelViewer*);

private:
    void rightButtonDown( const QPoint&) override;
    void leftDoubleClick( const QPoint&) override;
    void leftDrag( const QPoint&) override;
    void leftButtonDown( const QPoint&) override;
    void leftButtonUp( const QPoint&) override;

    FaceModelViewer* _viewer;
    bool _onDownCamLocked;
    void setCentre( const QPoint&);
};  // end class

}   // end namespace
}   // end namespace

#endif
