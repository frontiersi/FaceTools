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
    assert(vis);
    setCheckable( true, false);
}   // end ctor


bool ActionVisualise::checkState( Event e)
{
    FV* fv = MS::selectedView();
    if ( !fv)
        return false;

    if ( !_vis->isAvailable( fv, &primedMousePos()))
    {
        if ( _vis->isVisible( fv))
            _vis->setVisible( fv, false);
        return false;
    }   // end if

    if ( _vis->isVisible(fv))
    {
        _vis->refreshState( fv);
        return true;
    }   // end if

    return isTriggerEvent(e);
}   // end checkState


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

    FV* sfv = MS::selectedView();
    assert( sfv);
    FVS fvs;
    fvs.insert(sfv);

    if ( _vis->applyToAllViewers())
    {
        fvs = sfv->data()->fvs();
        _evg |= Event::ALL_VIEWERS;
    }   // end if

    if ( _vis->applyToAllInViewer())
    {
        FVS afvs;
        for ( const FV* fv : fvs)
            afvs.insert( fv->viewer()->attached());
        fvs.insert(afvs);
        _evg |= Event::ALL_VIEWS;
    }   // end if

    QPoint mc = primedMousePos();
    for ( FV* fv : fvs)
        if ( _vis->isAvailable( fv, &mc))
            _toggleVis( fv, &mc);
}   // end doAction


Event ActionVisualise::doAfterAction( Event)
{
    if ( isChecked())
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return _evg;
}   // end doAfterAction


void ActionVisualise::purge( const FM* fm)
{
    for ( FV* fv : fm->fvs())
        fv->purge( _vis);
}   // end purge
