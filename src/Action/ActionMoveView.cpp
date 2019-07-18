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

#include <ActionMoveView.h>
#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
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
    const FV* fv = MS::selectedView();
    return fv && _tviewer != fv->viewer() && (!_sviewer || _sviewer->attached().has(fv));
}   // end checkEnabled


// protected
void ActionMoveView::doAction( Event)
{
    FV* fv = MS::selectedView();
    MS::setSelected( nullptr); // De-select source FaceView

    const FM* fm = fv->data();
    // Does FV on target viewer have same model? Remove to replace if so.
    if ( _tviewer->get(fm))
        MS::removeFaceView( _tviewer->get(fm));

    // If the target viewer has other FaceViews, their visualisations
    // are applied to the FaceView being moved in (if possible).
    if ( !_tviewer->attached().empty())
        fv->copyFrom( _tviewer->attached().first());

    fv->setViewer(_tviewer);    // Attach to target viewer (detaches from source)
    MS::setSelected( fv);
    emit onEvent( Event::VIEWER_CHANGE);
}   // end doAction
