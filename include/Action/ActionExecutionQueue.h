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

#ifndef FACE_TOOLS_ACTION_EXECUTION_QUEUE_H
#define FACE_TOOLS_ACTION_EXECUTION_QUEUE_H

#include "FaceAction.h"
#include <unordered_map>
#include <list>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionExecutionQueue
{
public:
    // Test whether the given FaceAction should be added to the execution queue.
    void testPush( FaceAction*, const ChangeEventSet*);

    // Pop the next action to be worked on - setting pflag on return with the
    // value to be passed to FaceAction::process. Returns null if no more actions.
    FaceAction* pop( bool &pflag);

    size_t size() const { return _queue.size();}

private:
    std::list<FaceAction*> _queue;
    std::unordered_map<FaceAction*, bool> _actions;
};  // end class

}   // end namespace
}   // end namespace

#endif
