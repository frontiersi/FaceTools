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

#ifndef FACE_TOOLS_EVENT_PROCESS_RESPONSE_H
#define FACE_TOOLS_EVENT_PROCESS_RESPONSE_H

#include <FaceViewSet.h>

namespace FaceTools {
namespace Action {

class EventProcessResponse
{
public:
    EventProcessResponse()
        : _eid(NULL_EVENT),
          _rpred([](FVS&){return true;}),
          _fpred([](const FVS&){return false;}),
          _granted(false) {}

    EventProcessResponse( EventId e, bool flag)
        : _eid(e),
          _rpred([](FVS&){return true;}),
          _fpred([=](const FVS&){return flag;}),
          _granted(false) {}

    EventProcessResponse( EventId e, const ProcessFlagPredicate& fp)
        : _eid(e),
          _rpred([](FVS&){return true;}),
          _fpred(fp),
          _granted(false) {}

    EventProcessResponse( EventId e, const ResponsePredicate& rp=[](FVS&){return true;}, bool flag=true)
        : _eid(e),
          _rpred(rp),
          _fpred([=](const FVS&){return flag;}),
          _granted(false) {}

    EventProcessResponse( EventId e, const ResponsePredicate& rp, const ProcessFlagPredicate& fp)
        : _eid(e),
          _rpred(rp),
          _fpred(fp),
          _granted(false) {}

    // Return the event this process response is for.
    inline const EventId& event() const { return _eid;}

    // Should this event be granted a response according to the internal response predicate?
    inline bool grantResponse( FVS& fvs) const { return _granted = _rpred(fvs);}

    // Return the required process flag for a response (should grantResponse return true).
    inline bool processFlag( const FVS& fvs) const { return _granted && _fpred(fvs);}

private:
    EventId _eid;
    ResponsePredicate _rpred;
    ProcessFlagPredicate _fpred;
    mutable bool _granted;
};  // end struct

}   // end namespace
}   // end namespace

#endif
