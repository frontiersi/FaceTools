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

#ifndef FACE_TOOLS_FACE_HOVERING_INTERACTOR_H
#define FACE_TOOLS_FACE_HOVERING_INTERACTOR_H

/**
 * Uses ModelEntryExitInteractor to enable self only when mouse is over a model being
 * visualised by the BaseVisualisation passed to the constructor (visualisation must be
 * applied to the hovered over model).
 * Note that functions enteringModel and leavingModel are provided for descendent
 * interactors since, for example, events like onLeftButtonUp will not be sent if the
 * left button is down and the mouse is dragged off the model.
 */

#include "ModelEntryExitInteractor.h"
#include <BaseVisualisation.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT FaceHoveringInteractor : public ModelViewerInteractor 
{ Q_OBJECT
public:
    FaceHoveringInteractor( MEEI*, Vis::BaseVisualisation*, QStatusBar *sbar=nullptr);

    // Return the model being hovered over (null if nothing being hovered over).
    // It is guaranteed that this will return a non-null pointer if called from
    // within overridden implementations of enteringModel and leavingModel.
    inline Vis::FV* hoverModel() const { return _meei->model();}

protected:
    // Calling hoverModel() from within overridden implementations of these functions
    // will always return a model (either the one being entered, or the one being left).
    virtual void enteringModel(){}  // Called immediately before enabling this interactor.
    virtual void leavingModel(){}   // Called immediately after disabling this interactor.

private slots:
    void doOnEnterModel( Vis::FV*);
    void doOnLeaveModel();

private:
    MEEI *_meei;
    Vis::BaseVisualisation *_vis;
};  // end class

}   // end namespace
}   // end namespace

#endif
