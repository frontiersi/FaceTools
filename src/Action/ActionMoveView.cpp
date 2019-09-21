/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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


bool ActionMoveView::checkEnable( Event)
{
    if ( !MS::isViewSelected())
        return false;

    const FV* fv = MS::selectedView();

    // If moving the FaceView would leave the centre viewer empty, then don't enable.
    if ( fv->viewer() == MS::defaultViewer() && MS::defaultViewer()->attached().size() == 1)
        return false;
   
    // Enable only if the target viewer isn't the FaceView's current viewer, and either
    // a source viewer isn't defined, or the source viewer is the selected FaceView's viewer. 
    return _tviewer != fv->viewer() && (!_sviewer || _sviewer->attached().has(fv));
}   // end checkEnabled


void ActionMoveView::move( FV* fv, FMV* tfmv)
{
    assert( fv);
    assert( tfmv);

    if ( fv->viewer() != tfmv)
    {
        const FM* fm = fv->data();
        // Does FV on target viewer have same model? Remove to replace if so.
        if ( tfmv->get(fm))
            MS::removeFaceView( tfmv->get(fm));

        // If the target viewer has other FaceViews, their visualisations
        // are applied to the FaceView being moved in (if possible).
        if ( !tfmv->attached().empty())
            fv->copyFrom( tfmv->attached().first());

        fv->setViewer( tfmv);    // Attach to target viewer (detaches from source)
    }   // end if
}   // end move


// protected
void ActionMoveView::doAction( Event)
{
    FV* fv = MS::selectedView();
    MS::setSelected( nullptr); // De-select source FaceView
    move( fv, _tviewer);
    MS::setSelected( fv);
    emit onEvent( Event::VIEWER_CHANGE);
}   // end doAction
