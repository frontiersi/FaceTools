/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionRedo.h>
using FaceTools::Action::ActionRedo;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


ActionRedo::ActionRedo( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _dname(dn)
{
    connect( &*UndoStates::get(), &UndoStates::onUpdated, [this](){ refreshState();});
}   // end ctor

bool ActionRedo::isAllowed( Event)
{
    const bool cando = UndoStates::canRedo();
    QString dname = _dname;
    QString ttip = toolTip();
    if ( cando)
    {
        dname = QString("%1 '%2'").arg(_dname).arg( UndoStates::redoActionName());
        ttip = dname;
    }   // end if
    setDisplayName( dname);
    setToolTip( ttip);
    return cando;
}   // end isAllowed


bool ActionRedo::doBeforeAction( Event)
{
    // Need to save the name of the action being redone here since upon actually doing
    // the redo, signal onUpdated is emitted which causes isAllowed to execute which
    // will update the name of the redo action that would otherwise be shown in the
    // status bar by the default implementation of doAfterAction.
    _rname = UndoStates::redoActionName();
    MS::showStatus( QString("Redoing '%1'").arg(_rname));
    return true;
}   // end doBeforeAction


void ActionRedo::doAction( Event)
{
    _e = UndoStates::redo();
}   // end doAction


Event ActionRedo::doAfterAction( Event)
{
    MS::showStatus( QString("Finished redoing '%1'").arg(_rname), 3000);
    return _e;
}   // end doAfterAction
