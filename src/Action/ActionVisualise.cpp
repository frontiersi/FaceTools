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

#include <Action/ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionVisualise::ActionVisualise( const QString& dn, const QIcon& ic, BaseVisualisation* vis, const QKeySequence& ks)
    : FaceAction( dn, ic, ks), _vis(vis)
{
    assert(vis);
    setCheckable( true, false);
}   // end ctor


// private
bool ActionVisualise::isVisAvailable( const FV* fv) const
{
    bool isa = false;
    const FM* fm = fv ? fv->data() : nullptr;
    if ( fm)
    {
        fm->lockForRead();
        QPoint mpos = primedMousePos();
        isa = _vis->isAvailable( fv, &mpos);
        fm->unlock();
    }   // end if
    return isa;
}   // end isVisAvailable


bool ActionVisualise::checkState( Event e)
{
    const FV* fv = MS::selectedView();
    if ( fv && _vis->isVisible(fv))
    {
        _vis->checkState( fv);
        return true;
    }   // end if
    return fv && isTriggerEvent(e) && isVisAvailable(fv);
}   // end checkState


bool ActionVisualise::checkEnable( Event)
{
    const FV* fv = MS::selectedView();
    return isChecked() || ( fv && isVisAvailable( fv));
}   // end checkEnabled


// private
bool ActionVisualise::toggleVis( FV* fv, const QPoint* mc)
{
    const bool appliedPre = _vis->isVisible(fv);
    if ( isChecked() || !_vis->isToggled())
        fv->apply( _vis, mc);
    else
        _vis->setVisible( fv, false);
    // Returns true if there was a change in the visibility of the visualisation.
    return appliedPre != _vis->isVisible(fv);
}   // end toggleVis


void ActionVisualise::doAction( Event /*e*/)
{
    //std::cerr << "ActionVisualise::doAction on event [ " << EventChecker(e).name() << " ]" << std::endl;
    int xev = int(Event::NONE);

    FV* sfv = MS::selectedView();
    FVS fvs;
    fvs.insert(sfv);

    if ( _vis->applyToSelectedModel())
    {
        fvs = sfv->data()->fvs();
        xev = int(Event::ALL_VIEWS);
    }   // end if

    if ( _vis->applyToAllInViewer())
    {
        FVS afvs;
        for ( const FV* fv : fvs)
            afvs.insert( fv->viewer()->attached());
        fvs.insert(afvs);
        xev |= int(Event::ALL_VIEWERS);
    }   // end if

    QPoint mc = primedMousePos();
    for ( FV* fv : fvs)
    {
        if ( _vis->isAvailable( fv, &mc))
            toggleVis( fv, &mc);
    }   // end for

    emit onEvent( Event(int(Event::VIEW_CHANGE) | xev));
}   // end doAction


void ActionVisualise::purge( const FM* fm, Event e)
{
    for ( FV* fv : fm->fvs())
        fv->purge( _vis, e);  // Calls _vis->purge(fv, e)
}   // end purge
