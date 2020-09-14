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
using MS = FaceTools::Action::ModelSelector;


ActionVisualise::ActionVisualise( const QString& dn, const QIcon& ic, BaseVisualisation* vis, const QKeySequence& ks)
    : FaceAction( dn, ic, ks), _vis(vis)
{
    setCheckable( true, false);
}   // end ctor


FaceTools::FVS ActionVisualise::_getWorkViews()
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
    }   // end sfv

    return fvs;
}   // end _getWorkViews


bool ActionVisualise::update( Event e)
{
    bool rval = false;
    FVS fvs = _getWorkViews();
    const bool isTrigger = isTriggerEvent(e);
    for ( FV *fv : fvs)
    {
        if ( _vis->isAvailable( fv, &primedMousePos()) && (_vis->isVisible(fv) || isTrigger))
        {
            rval = true;
            if ( _vis->isVisible(fv))   // Refresh if already visible
                _vis->refresh( fv);
        }   // end if
        else if ( _vis->isVisible( fv))
            _vis->setVisible( fv, false);   // Hide visualisation
    }   // end for

    if ( !fvs.empty())
        MS::updateRender();

    return rval;
}   // end update


bool ActionVisualise::isAllowed( Event)
{
    const FV* fv = MS::selectedView();
    return fv && _vis->isAvailable( fv, &primedMousePos());
}   // end isAllowed


void ActionVisualise::_toggleVis( FV* fv, const QPoint* mc) const
{
    if ( isChecked() || !_vis->isToggled())
        fv->apply( _vis, mc);
    else
        _vis->setVisible( fv, false);
}   // end _toggleVis


void ActionVisualise::doAction( Event)
{
    _evg = Event::VIEW_CHANGE;
    FVS fvs = _getWorkViews();
    const QPoint& mc = primedMousePos();
    for ( FV* fv : fvs)
        if ( _vis->isAvailable( fv, &mc))
            _toggleVis( fv, &mc);
}   // end doAction


Event ActionVisualise::doAfterAction( Event e)
{
    if ( isChecked())
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    if ( e == Event::USER && !isTriggerEvent(e))
        MS::showStatus( displayName() + (isChecked() ? " ON" : " OFF"), 5000);
    return _evg;
}   // end doAfterAction


void ActionVisualise::purge( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        fv->purge( _vis);
}   // end purge
