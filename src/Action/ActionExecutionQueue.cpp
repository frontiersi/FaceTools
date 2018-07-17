/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <ActionExecutionQueue.h>
#include <cassert>
using FaceTools::Action::ActionExecutionQueue;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;


// public
void ActionExecutionQueue::testPush( FaceAction* act, const ChangeEventSet* cs)
{
    const bool ispresent = _actions.count(act) > 0; // Is the action already in the queue?
    bool add2q = false;
    bool pflag = false;
    if ( ispresent)
        pflag = _actions.at(act);

    // Check if the process flag should be true against the set of received change events.
    if ( !pflag)
    {
        const ChangeEventSet* aset = &act->processEvents(); // Set of changes requiring the action to be processed
        // Are any of the change events in the given set specified by the action
        // as a trigger for processing it? If so, find and set the associated process
        // flag and ensure that the add2q indicator is set.
        for ( auto c : *cs)
        {
            if ( aset->count(c) > 0)
            {
                pflag = pflag || aset->find(c)->processFlag;
                _actions[act] = pflag;  // Update the process flag
                add2q = true;
                if ( pflag) // Break out as soon as pflag is true
                    break;
            }   // end if
        }   // end for
    }   // end if

    if ( add2q && !ispresent)   // Don't add if already present
        _queue.push_back(act);
}   // end testPush


FaceAction* ActionExecutionQueue::pop( bool& pflag)
{
    FaceAction* act = nullptr;
    if ( !_queue.empty())
    {
        act = *_queue.begin();
        pflag = _actions.at(act);  // Set the process flag
        _queue.pop_front();
    }   // end if

    // DO NOT remove actions from _actions until the queue is empty!
    if ( _queue.empty())
        _actions.clear();
    return act;
}   // end pop
