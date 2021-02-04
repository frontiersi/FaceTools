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

#include <Action/ActionSetParallelProjection.h>
#include <FileIO/FaceModelManager.h>
using FaceTools::Action::ActionSetParallelProjection;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;


ActionSetParallelProjection::ActionSetParallelProjection( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _ev(Event::NONE)
{
    setCheckable(true,false);
    addTriggerEvent( Event::CLOSED_MODEL);
    addRefreshEvent( Event::CAMERA_CHANGE);
}   // end ctor


bool ActionSetParallelProjection::update( Event e)
{
    return MS::defaultViewer()->parallelProjection() && !has( e, Event::LOADED_MODEL);
}   // end update


bool ActionSetParallelProjection::isAllowed( Event e) { return FMM::numOpen() > 0;}


void ActionSetParallelProjection::doAction( Event)
{
    _ev = Event::NONE;
    if ( setParallelProjection( isChecked() && FMM::numOpen() > 0))
        _ev = Event::CAMERA_CHANGE;
}   // end doAction


bool ActionSetParallelProjection::setParallelProjection( bool enable)
{
    bool changed = false;
    for ( FMV *fmv : MS::viewers())
    {
        if ( fmv->parallelProjection() != enable)
        {
            fmv->setParallelProjection( enable);
            changed = true;
        }   // end if
    }   // end for
    return changed;
}   // end setParallelProjection


Event ActionSetParallelProjection::doAfterAction( Event) { return _ev;}
