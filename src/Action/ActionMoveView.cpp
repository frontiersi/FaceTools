/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <cassert>
using FaceTools::Action::ActionMoveView;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionMoveView::ActionMoveView( FMV *tv, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tvwr(tv), _svwr(sv)
{
    assert( _tvwr);
    addRefreshEvent( Event::VIEWER_CHANGE);
    if ( MS::defaultViewer() == _tvwr)
        addTriggerEvent( Event::CLOSED_MODEL);
}   // end ctor


bool ActionMoveView::isAllowed( Event)
{
    const bool isSrcVwr = MS::defaultViewer() == _svwr;
    const FV* fv = MS::selectedView();
    // Disallow if moving away from the centre viewer leaves it empty unless the target has views.
    if ( !fv || (isSrcVwr && _svwr->attached().size() == 1 && _tvwr->empty()))
        return false;
    // Enable only if the target viewer isn't the FaceView's current viewer, and either
    // a source viewer isn't defined, or the source viewer is the selected FaceView's viewer. 
    const bool allow = _tvwr != fv->viewer() && (!_svwr || _svwr->has(fv));
    // Restrict maximum number of model views per viewer to 2 unless moving
    // this view to the target viewer will cause views of the same model to merge.
    return allow && (_tvwr->attached().size() < 2 || _tvwr->get(fv->data()));
}   // end isAllowed


bool ActionMoveView::doBeforeAction( Event e) { return isAllowed(e);}


void ActionMoveView::doAction( Event)
{
    FV *fv = MS::selectedView();
    FM::RPtr fm = fv->rdata();
    if ( _tvwr->get(fm.get())) // Remove target viewer's copy of the model if present.
        MS::remove( _tvwr->get(fm.get()));

    _tvwr->setUpdatesEnabled(false);
    _svwr->setUpdatesEnabled(false);
    fv->setViewer( _tvwr);
    _tvwr->setCamera( _svwr->camera());
    if ( MS::defaultViewer()->empty())
    {
        assert( MS::defaultViewer() == _svwr);
        // Move all views in the target viewer back to the source (default) viewer
        while ( !_tvwr->empty())
            _tvwr->attached().first()->setViewer( _svwr);
        _svwr->setSelected(fv);  // Ensure the current view is still the selected one
    }   // end if

    _tvwr->setUpdatesEnabled(true);
    _svwr->setUpdatesEnabled(true);
}   // end doAction


Event ActionMoveView::doAfterAction( Event) { return Event::VIEWER_CHANGE;}
