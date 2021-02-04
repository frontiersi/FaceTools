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

#include <Action/ActionVisualise.h>
#include <FaceModelViewer.h>
#include <Vis/FaceView.h>
#include <FaceModel.h>
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionVisualise::ActionVisualise( const QString& dn, const QIcon& ic, BaseVisualisation* vis, const QKeySequence& ks)
    : FaceAction( dn, ic, ks), _vis(vis)
{
    // Required for visualisations to check their exclusivity
    addRefreshEvent( Event::VIEW_CHANGE | Event::MESH_CHANGE | Event::VIEWER_CHANGE);
    setCheckable( true, false);
}   // end ctor


FaceTools::FVS ActionVisualise::_getWorkViews( Event e)
{
    FV* sfv = MS::selectedView();
    FVS fvs;
    if ( sfv)
    {
        if ( _vis->applyToAllViewers())
        {
            _evg |= Event::ALL_VIEWERS;
            fvs = sfv->data()->fvs();
        }   // end if
        else
            fvs.insert(sfv);

        if ( _vis->applyToAllInViewer())
        {
            FVS afvs;
            for ( const FV* fv : fvs)
                afvs.insert( fv->viewer()->attached());
            fvs.insert(afvs);
            _evg |= Event::ALL_VIEWS;
        }   // end if

        // If the event included VIEWER_CHANGE, include views in the previous viewer.
        if ( has( e, Event::VIEWER_CHANGE) && sfv->pviewer())
            fvs.insert( sfv->pviewer()->attached());
    }   // end if

    return fvs;
}   // end _getWorkViews


bool ActionVisualise::update( Event e)
{
    const bool doRefresh = any(refreshEvents(), e) && !has( e, Event::VIEW_CHANGE);
    bool rval = false;
    FVS fvs = _getWorkViews( e);
    for ( FV *fv : fvs)
    {
        if ( !_vis->isVisible(fv))
            continue;

        if ( !_vis->isAvailable( fv))
        {
            _vis->setVisible( fv, false);   // Hide visualisation
            continue;
        }   // end if

        rval = true;
        if ( doRefresh)
            _vis->refresh(fv);
    }   // end for

    return rval;
}   // end update


bool ActionVisualise::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && _vis->isAvailable( fv);
}   // end isAllowed


void ActionVisualise::doAction( Event e)
{
    const bool turnOn = any( triggerEvents(), e) || isChecked();
    _evg = Event::VIEW_CHANGE;
    FVS fvs = _getWorkViews( e);
    for ( FV* fv : fvs)
    {
        if ( turnOn && _vis->isAvailable( fv))
            fv->apply( _vis);
        else
            _vis->setVisible( fv, false);
    }   // end for
}   // end doAction


Event ActionVisualise::doAfterAction( Event e)
{
    const FV *fv = MS::selectedView();
    const bool isvis = fv && _vis->isVisible( fv);
    if ( e == Event::USER && !any( triggerEvents(), e))
        MS::showStatus( displayName() + (isvis ? " ON" : " OFF"), 5000);
    return _evg;
}   // end doAfterAction


void ActionVisualise::purge( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        fv->purge( _vis);
}   // end purge
