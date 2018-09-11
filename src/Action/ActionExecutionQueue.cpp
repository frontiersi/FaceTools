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

#include <ActionExecutionQueue.h>
#include <EventProcessResponse.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionExecutionQueue;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventSet;
using FaceTools::Action::EPR;
using FaceTools::FVS;


// public
bool ActionExecutionQueue::push( FaceAction* act, const FVS& fvs, const EventSet& E)
{
    if (_actions.count(act) > 0)    // Don't push an action that's already on the queue.
        return false;

    const size_t qsize = _queue.size();
    assert(act);

    // Get all event responses registered for the action.
    std::unordered_set<const EPR*> eprs;
    for ( EventId e : E)
    {
        const EventProcessResponse* response = act->eventResponse(e);
        if ( response)
            eprs.insert(response);
    }   // end for

    // Of these, choose the response one that maximises the size of the FaceViewSet.
    const EPR* bestResponse = nullptr;
    FVS mfvs;   // Initially empty
    bool pflag = true;
    for ( const EPR* response : eprs)
    {
        FVS nfvs = fvs; // Copy out since in granting the response, the contents may change
        const bool granted = response->grantResponse( nfvs);
        if ( granted && nfvs.size() >= mfvs.size())
        {
            bestResponse = response;
            mfvs = nfvs;
            pflag = pflag && response->processFlag( mfvs);  // process flag must be true over ALL granted responses to stay true
        }   // end if
    }   // end for

    if ( bestResponse)
    {
        _queue.push_back( {act, FVS::create(mfvs), pflag});
        _actions.insert(act);
        std::cerr << " Q<<-- : " << act->dname() << " <" << act << "> (#FV = " << mfvs.size() << ")" << std::endl;
    }   // end if
    return _queue.size() > qsize;
}   // end push


// public
FaceAction* ActionExecutionQueue::pop( bool& pflag)
{
    FaceAction* nact = nullptr;

    while ( !_queue.empty() && !nact)
    {
        auto a = *_queue.begin();
        _queue.pop_front();

        FaceAction* tact = a.act;
        pflag = a.flag;

        std::cerr << " Q-->> : " << tact->dname() << " <" << tact << ">";

        tact->clearReady();
        tact->setReady( *a.fvs, true);
        if ( tact->isEnabled())
            nact = tact;
        else if ( tact->isCheckable() && tact->isChecked() != tact->defaultCheckState())
        {
            // Action not enabled, but if checkable and not in default check state, it must be set to the default state.
            pflag = tact->defaultCheckState();
            nact = tact;
        }   // end else
        else
            std::cerr << " ** UNAVAILABLE ** (ready set size = " << tact->ready().size() << ")";
        std::cerr << std::endl;
    }   // end while

#ifndef NDEBUG
    if (!nact)
        assert(_queue.empty());
#endif

    if ( _queue.empty())
        _actions.clear();

    return nact;
}   // end pop
