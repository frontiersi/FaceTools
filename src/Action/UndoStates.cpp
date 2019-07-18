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

#include <UndoStates.h>
#include <FaceAction.h>
#include <QThread>
#include <cassert>
using FaceTools::Action::UndoStates;
using FaceTools::Action::UndoState;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventGroup;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

// static init
UndoStates::Ptr UndoStates::_singleton;


UndoStates::Ptr UndoStates::get()
{
    if ( !_singleton)
        _singleton = Ptr( new UndoStates);
    return _singleton;
}   // end get


void UndoStates::clear( const FM* fm) { get()->_clear(fm);}
void UndoStates::_clear( const FM* fm) { _stacks.erase(fm);}


void UndoStates::clear() { get()->_clear();}
void UndoStates::_clear() { _stacks.clear();}


void UndoStates::storeUndo( const FaceAction* a, EventGroup egrp, bool autoRestore) { get()->_storeUndo(a, egrp, autoRestore);}
void UndoStates::_storeUndo( const FaceAction* a, EventGroup egrp, bool autoRestore)
{
#ifndef NDEBUG
    std::cerr << "UndoStates::storeUndo: thread ID = " << QThread::currentThreadId() << std::endl;
#endif
    UndoState::Ptr us;
    if ( autoRestore)
        us = UndoState::create( egrp, autoRestore);
    else
    {
        us = a->makeUndoState();
        if ( !us)
            return;
    }   // end else

    us->_action = const_cast<FaceAction*>(a);

    _mutex.lockForWrite();
    Stacks& stacks = _stacks[us->_fm];
    if ( stacks.undos.size() == MAX_RESTORES)
        stacks.undos.pop_back();
    stacks.undos.push_front( us);   // Push to undo stack
    stacks.redos.clear(); // Clear the redo stack
    _mutex.unlock();
    emit onUpdated();
}   // end _storeUndo


bool UndoStates::canUndo() { return get()->_canUndo();}
bool UndoStates::_canUndo()
{
    const FM* fm = MS::selectedModel();
    _mutex.lockForRead();
    const bool notEmpty = _stacks.count(fm) > 0 && !_stacks.at(fm).undos.empty();
    _mutex.unlock();
    return notEmpty;
}   // end _canUndo


bool UndoStates::canRedo() { return get()->_canRedo();}
bool UndoStates::_canRedo()
{
    const FM* fm = MS::selectedModel();
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
        aname = _stacks.at(fm).undos.front()->_action->displayName();
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
        aname = _stacks.at(fm).redos.front()->_action->displayName();
    _mutex.unlock();
    return aname;
}   // end _redoActionName


void UndoStates::undo() { get()->_undo();}
void UndoStates::_undo()
{
    assert( canUndo());

    _mutex.lockForWrite();
    Stacks& stacks = _stacks.at(MS::selectedModel());
    UndoState::Ptr ustate = stacks.undos.front();
    stacks.undos.pop_front();

    // Before restoring state, we save the current state for redo purposes
    UndoState::Ptr rstate;
    if ( ustate->_autoRestore)
        rstate = UndoState::create( ustate->_egrp, true);
    else
    {
        rstate = ustate->_action->makeUndoState();
        if ( !rstate)
            return;
    }   // end else

    rstate->_action = ustate->_action;
    rstate->_egrp = ustate->_egrp;
    stacks.redos.push_front(rstate);
    _mutex.unlock();

    ustate->restore();
    emit get()->onUpdated();
}   // end _undo


void UndoStates::redo() { get()->_redo();}
void UndoStates::_redo()
{
    assert( canRedo());

    _mutex.lockForWrite();
    Stacks& stacks = _stacks.at(MS::selectedModel());
    UndoState::Ptr rstate = stacks.redos.front();
    stacks.redos.pop_front();

    // Before restoring state, we save the current state for undo purposes
    UndoState::Ptr ustate;
    if ( rstate->_autoRestore)
        ustate = UndoState::create( rstate->_egrp, true);
    else
    {
        ustate = rstate->_action->makeUndoState();
        if ( !ustate)
            return;
    }   // end else

    ustate->_action = rstate->_action;
    ustate->_egrp = rstate->_egrp;
    stacks.undos.push_front(ustate);
    _mutex.unlock();

    rstate->restore();
    emit get()->onUpdated();
}   // end _redo
