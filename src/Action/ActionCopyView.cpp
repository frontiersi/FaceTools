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

#include <Action/ActionCopyView.h>
#include <Vis/FaceView.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Action::ActionCopyView;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionCopyView::ActionCopyView( FMV *tv, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _sviewer(sv)
{
    addRefreshEvent( Event::VIEWER_CHANGE);
}   // end ctor


bool ActionCopyView::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    // Allowed if data not already on the target viewer
    bool allow = fv && !_tviewer->isAttached(fv->data());
    if ( allow && _sviewer != nullptr)
        allow = _sviewer->has(fv);
    // Restrict maximum number of model views to 2
    return allow && fv->data()->fvs().size() < 2;
}   // end isAllowed


void ActionCopyView::doAction( Event)
{
    FV* sfv = MS::selectedView();
    MS::setSelected(nullptr);

    FM* fm = sfv->data();
    FV* nfv = MS::add( fm, _tviewer); // Create the new FV from the underlying data.
    nfv->copyFrom( sfv);    // Copy over visualisations from the source to new face view

    MS::setSelected( nfv);
}   // end doAction


Event ActionCopyView::doAfterAction( Event)
{
    return Event::VIEWER_CHANGE | Event::MODEL_SELECT;
}   // end doAfterAction
