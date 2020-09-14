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

#include <Action/ActionCopyView.h>
#include <Vis/BaseVisualisation.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
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
    addRefreshEvent( Event::VIEWER_CHANGE);
}   // end ctor


bool ActionCopyView::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    bool allowed = fv && !_tviewer->isAttached(fv->data());   // Allowed if data not already on the target viewer
    if ( allowed && _sviewer != nullptr)
        allowed = _sviewer->attached().has(fv);
    return allowed;
}   // end isAllowed


void ActionCopyView::doAction( Event)
{
    FV* sfv = MS::selectedView();
    MS::setSelected(nullptr);

    FM* fm = sfv->data();
    FV* nfv = MS::add( fm, _tviewer); // Create the new FV from the underlying data.
    nfv->copyFrom( sfv); // Copy over visualisations from the source to new face view

    MS::setSelected( nfv);
}   // end doAction


Event ActionCopyView::doAfterAction( Event) { return Event::VIEWER_CHANGE;}
