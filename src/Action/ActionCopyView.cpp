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

#include <ActionCopyView.h>
#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionCopyView;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionCopyView::ActionCopyView( FMV *tv, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _sviewer(sv)
{
}   // end ctor


bool ActionCopyView::checkEnable( Event)
{
    const FV* fv = MS::selectedView();
    bool allowed = fv && !_tviewer->isAttached(fv->data());   // Allowed if data not already on the target viewer
    if ( allowed && _sviewer != nullptr)
        allowed = _sviewer->attached().has(fv);
    return allowed;
}   // end checkEnabled


void ActionCopyView::doAction( Event)
{
    FV* fv = MS::selectedView();
    // If there exist FVs in the target viewer, copy their visualisations
    // to the new FV. Otherwise, copy over visualisations from source FV.
    FV* cfv = _tviewer->attached().first();
    if ( cfv == nullptr)
        cfv = fv;

    FM* fm = fv->data();
    fm->lockForRead();
    FV* nfv = MS::addFaceView( fm, _tviewer); // Create the new FV from the underlying data.

    // Copy over visualisations (where available) to the target view if
    // they're visible as well as other state information.
    nfv->copyFrom( cfv);

    fm->unlock();

    MS::setSelected( nfv);
    emit onEvent( Event::VIEWER_CHANGE);
}   // end doAction
