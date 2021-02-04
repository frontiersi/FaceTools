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

#include <Action/UndoStates.h>
#include <Action/FaceAction.h>
#include <QThread>
#include <cassert>
using FaceTools::Action::UndoStates;
using FaceTools::Action::UndoState;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;

// static init
UndoStates::Ptr UndoStates::_singleton;


UndoStates::Ptr UndoStates::get()
{
    if ( !_singleton)
        _singleton = Ptr( new UndoStates);
    return _singleton;
}   // end get


void UndoStates::clear( const FM* fm) { get()->_clear(fm);}
void UndoStates::_clear( const FM* fm)
{
    assert(fm);
    _stacks.erase(fm);
}   // end _clear


void UndoStates::clear() { get()->_clear();}
void UndoStates::_clear() { _stacks.clear();}


void UndoStates::storeUndo( const FaceAction* a, Event e, bool autoRestore) { get()->_storeUndo(a, e, autoRestore);}
void UndoStates::_storeUndo( const FaceAction* a, Event e, bool autoRestore)
{
    UndoState::Ptr us = UndoState::create( a, e, autoRestore);
    if ( !autoRestore)
        a->saveState( *us);

    _mutex.lockForWrite();
    Stacks& stacks = _stacks[us->model()];
    if ( stacks.undos.size() == MAX_RESTORES)
        stacks.undos.pop_back();
    stacks.undos.push_front( us);   // Push to undo stack
    stacks.oldRedos = stacks.redos; // In case of scrapping - can roll back
    stacks.redos.clear(); // Clear the redo stack
    _mutex.unlock();
    emit onUpdated();
}   // end _storeUndo


void UndoStates::scrapLastUndo( const FM *fm) { get()->_scrapLastUndo( fm);}
void UndoStates::_scrapLastUndo( const FM *fm)
{
    assert( _canUndo( fm));
    _mutex.lockForWrite();
    Stacks& stacks = _stacks.at(fm);
    UndoState::Ptr ustate = stacks.undos.front();
    stacks.undos.pop_front();
    stacks.redos = stacks.oldRedos;
    stacks.oldRedos.clear();
    _mutex.unlock();
    emit onUpdated();
}   // end _scrapLastUndo


bool UndoStates::canUndo() { return get()->_canUndo( MS::selectedModel());}
bool UndoStates::_canUndo( const FM *fm)
{
    _mutex.lockForRead();
    const bool notEmpty = _stacks.count(fm) > 0 && !_stacks.at(fm).undos.empty();
    _mutex.unlock();
    return notEmpty;
}   // end _canUndo


bool UndoStates::canRedo() { return get()->_canRedo( MS::selectedModel());}
bool UndoStates::_canRedo( const FM *fm)
{
    _mutex.lockForRead();
    const bool notEmpty = _stacks.count(fm) > 0 && !_stacks.at(fm).redos.empty();
    _mutex.unlock();
    return notEmpty;
}   // end _canRedo


QString UndoStates::undoActionName() { return get()->_undoActionName();}
QString UndoStates::_undoActionName()
{
    const FM* fm = MS::selectedModel();
    QString aname = "";
    _mutex.lockForRead();
    if ( _stacks.count(fm) > 0)
        aname = _stacks.at(fm).undos.front()->name();
    _mutex.unlock();
    return aname;
}   // end _undoActionName


QString UndoStates::redoActionName() { return get()->_redoActionName();}
QString UndoStates::_redoActionName()
{
    const FM* fm = MS::selectedModel();
    QString aname = "";
    _mutex.lockForRead();
    if ( _stacks.count(fm) > 0)
        aname = _stacks.at(fm).redos.front()->name();
    _mutex.unlock();
    return aname;
}   // end _redoActionName


Event UndoStates::undo() { return get()->_undo( MS::selectedModel());}
Event UndoStates::_undo( const FM *fm)
{
    assert( _canUndo( fm));

    _mutex.lockForWrite();
    Stacks& stacks = _stacks.at(fm);
    UndoState::Ptr ustate = stacks.undos.front();
    stacks.undos.pop_front();

    // Before restoring state, we save the current state for redo purposes
    UndoState::Ptr rstate = UndoState::create( ustate->action(), ustate->events(), ustate->isAutoRestore());
    if ( !ustate->isAutoRestore())
        ustate->action()->saveState( *rstate);
    stacks.redos.push_front( rstate);
    stacks.oldRedos.clear();
    _mutex.unlock();

    Event e = ustate->restore();
    emit get()->onUpdated();
    return e;
}   // end _undo


Event UndoStates::redo() { return get()->_redo( MS::selectedModel());}
Event UndoStates::_redo( const FM *fm)
{
    assert( _canRedo( fm));

    _mutex.lockForWrite();
    Stacks& stacks = _stacks.at( fm);
    UndoState::Ptr rstate = stacks.redos.front();
    stacks.redos.pop_front();
    // Before restoring state, we save the current state for undo purposes
    UndoState::Ptr ustate = UndoState::create( rstate->action(), rstate->events(), rstate->isAutoRestore());
    if ( !rstate->isAutoRestore())
        rstate->action()->saveState( *ustate);
    stacks.undos.push_front( ustate);
    _mutex.unlock();

    Event e = rstate->restore();
    emit get()->onUpdated();
    return e;
}   // end _redo
