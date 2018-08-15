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

#include <FaceHoveringInteractor.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Interactor::FaceHoveringInteractor;
using FaceTools::Interactor::ModelViewerInteractor;


// public
FaceHoveringInteractor::FaceHoveringInteractor( FEEI *feei, FaceTools::Vis::BaseVisualisation *vis, QStatusBar* sbar)
    : ModelViewerInteractor( nullptr, sbar), _feei(feei), _vis(vis)
{
    assert(vis);
    assert(feei);
    // Use the FaceEntryExitInteractor to ensure this interactor is enabled only when moving over a face.
    connect( feei, &FEEI::onEnterModel, this, &FaceHoveringInteractor::doOnEnterModel);
    connect( feei, &FEEI::onLeaveModel, this, &FaceHoveringInteractor::doOnLeaveModel);
    setEnabled(false);
}   // end ctor


// private slot
void FaceHoveringInteractor::doOnEnterModel( FaceTools::FaceControl* fc)
{
    this->enteringModel();
    setEnabled( _vis->isApplied(fc));
}   // end doOnEnterModel


// private slot
void FaceHoveringInteractor::doOnLeaveModel()
{
    setEnabled(false);
    this->leavingModel();
}   // end doOnLeaveModel
