/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionMoveView.h>
#include <Vis/BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMoveView;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionMoveView::ActionMoveView( FMV *tv, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _sviewer(sv)
{
    assert( _tviewer);
}   // end ctor


bool ActionMoveView::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    // Disallow if moving away from the centre viewer would leave it empty - UNLESS
    // moving the view merges it with a copy of the model on the target viewer.
    if ( !fv || (MS::defaultViewer() == fv->viewer() && fv->viewer()->attached().size() == 1 && !_tviewer->isAttached(fv->data())))
        return false;
    // Enable only if the target viewer isn't the FaceView's current viewer, and either
    // a source viewer isn't defined, or the source viewer is the selected FaceView's viewer. 
    return _tviewer != fv->viewer() && (!_sviewer || _sviewer->attached().has(fv));
}   // end isAllowed


void ActionMoveView::move( FV* fv, FMV* tfmv)
{
    assert( fv);
    assert( tfmv);

    assert( fv->viewer() != tfmv);

    const FM* fm = fv->data();
    // Does target viewer have view of same model? Remove to replace if so.
    if ( tfmv->get(fm))
        MS::remove( tfmv->get(fm));

    fv->setViewer( tfmv);    // Attach to target viewer (detaches from source)
    // If moving the view has left the centre viewer empty, then move it back to the centre viewer.
    if ( MS::defaultViewer()->attached().empty())
        fv->setViewer( MS::defaultViewer());
}   // end move


void ActionMoveView::doAction( Event) { move( MS::selectedView(), _tviewer);}


Event ActionMoveView::doAfterAction( Event) { return Event::VIEWER_CHANGE;}
