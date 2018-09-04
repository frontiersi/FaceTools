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

#ifndef FACE_TOOLS_ACTION_EXECUTION_QUEUE_H
#define FACE_TOOLS_ACTION_EXECUTION_QUEUE_H

#include "FaceAction.h"
#include <list>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionExecutionQueue
{
public:
    ActionExecutionQueue() {}

    // Push the given action to the queue along with the set of Vis::FVs to
    // work on if any member of the given event set is an initiator for
    // the action. The process flag for the action will be in agreement
    // with all initiating events for the action. Returns true if the
    // action was pushed onto the queue.
    bool push( FaceAction*, const FVS&, const EventSet&);

    // Get the next action from the queue or null if there's no next action.
    // On return of non-null, pflag is set to the check state value for the
    // returned action's process function.
    FaceAction* pop( bool &pflag);

    inline bool empty() const { return _queue.empty();}

private:
    struct QElem
    {
        FaceAction* act;
        FVS::Ptr fvs;
        bool flag;
    };  // end struct
    std::list<QElem> _queue;
    std::unordered_set<FaceAction*> _actions;

    ActionExecutionQueue( const ActionExecutionQueue&) = delete;
    void operator=( const ActionExecutionQueue&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
