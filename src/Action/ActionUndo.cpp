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

#include <Action/ActionUndo.h>
using FaceTools::Action::ActionUndo;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


ActionUndo::ActionUndo( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _dname(dn)
{
    connect( &*UndoStates::get(), &UndoStates::onUpdated, [this](){ refreshState();});
}   // end ctor


bool ActionUndo::checkEnable( Event)
{
    const bool cando = UndoStates::canUndo();
    QString dname = _dname; // "Undo"
    QString ttip = toolTip();
    if ( cando)
    {
        dname = QString("%1 '%2'").arg(_dname).arg( UndoStates::undoActionName());
        ttip = dname;
    }   // end if
    setDisplayName( dname);
    setToolTip( ttip);
    return cando;
}   // end checkEnable


bool ActionUndo::doBeforeAction( Event)
{
    // Need to save the name of the action being undone here since upon actually doing
    // the undo, signal onUpdated is emitted which causes checkEnable to execute which
    // will update the name of the undo action that would otherwise be shown in the
    // status bar by the default implementation of doAfterAction.
    _rname = UndoStates::undoActionName();
    MS::showStatus( QString("Undoing '%1'").arg(_rname));
    return true;
}   // end doBeforeAction


void ActionUndo::doAction( Event)
{
    UndoStates::undo();
}   // end doAction


void ActionUndo::doAfterAction( Event)
{
    MS::showStatus( QString("Finished undoing '%1'").arg(_rname), 3000);
}   // end doAfterAction
