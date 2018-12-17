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

#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionVisualise::ActionVisualise( BaseVisualisation* vis, bool visOnLoad)
    : FaceAction( vis->getDisplayName(), vis->getIcon()), _vis(vis)
{
    assert(vis);
    setCheckable( true, false);
    setVisible( vis->isUIVisible());
    setPurgeOnEvent(GEOMETRY_CHANGE);

    if ( visOnLoad)
    {
        TestFVSTrue loadResponsePredicate = [=]( const FVS& fvs)
        {
            const FMS& fms = fvs.models();
            assert(fms.size() == 1);    // LOADED_MODEL events are only ever a single model.
            const FM* fm = *fms.begin();
            return vis->allowShowOnLoad( fm);
        };  // end loadResponsePredicate
        setRespondToEventIf( LOADED_MODEL, loadResponsePredicate, true);
    }   // end if
}   // end ctor


// private
bool ActionVisualise::isVisAvailable( const FM* fm) const
{
    fm->lockForRead();
    const bool isa = fm && _vis->isAvailable(fm);
    fm->unlock();
    return isa;
}   // end isVisAvailable


// private
bool ActionVisualise::isVisAvailable( const FV* fv, const QPoint* mc) const
{
    bool isa = false;
    const FM* fm = fv ? fv->data() : nullptr;
    if ( fm)
    {
        if ( !mc)
            isa = isVisAvailable( fm);
        else
        {
            fm->lockForRead();
            isa = _vis->isAvailable(fv, mc);
            fm->unlock();
        }   // end else
    }   // end if
    return isa;
}   // end isVisAvailable


// private
// Returns true iff the contents of the FaceViewSet matches the visualisation's specification
// for models/views, and the models/views meet the visualisation criteria.
bool ActionVisualise::isVisAvailable( const FVS& fvs, const QPoint* mc) const
{
    if ( fvs.empty())
        return false;

    bool allowed = false;
    if ( _vis->applyToSelectedModel())
    {
        const FMS& fms = fvs.models();
        allowed = std::all_of( std::begin(fms), std::end(fms), [this](const FM* fm){ return this->isVisAvailable(fm);});
    }   // end if
    else
        allowed = std::all_of( std::begin(fvs), std::end(fvs), [=](const FV* fv){ return this->isVisAvailable(fv, mc);});
    return allowed;
}   // end isVisAvailable


bool ActionVisualise::testReady( const FV* fv) { return fv && (fv->isApplied(_vis) || isVisAvailable(fv->data()));}


void ActionVisualise::tellReady( const FV* fv, bool v)
{
    if ( _vis->applyOnReady())
    {
        setChecked(v);
        toggleVis( const_cast<FV*>(fv), nullptr);
    }   // end if
}   // end tellReady


bool ActionVisualise::testEnabled( const QPoint* mc) const { return isVisAvailable( ready(), mc);}
bool ActionVisualise::testIfCheck( const FV* fv) const { return fv && fv->isApplied(_vis);}


// private
bool ActionVisualise::toggleVis( FV* fv, const QPoint* mc)
{
    const bool appliedPre = fv->isApplied(_vis);

    // If this visualisation isn't toggled, or it's exclusive, then the current exclusive visualisation must first be removed.
    if ( isExclusive())
    {
        if ( fv->exvis() != nullptr)
            fv->remove(fv->exvis());
    }   // end if

    if ( isChecked() || !_vis->isToggled())
        fv->apply( _vis, mc);
    else
        fv->remove( _vis);

    const bool appliedPost = fv->isApplied(_vis);
    setChecked( appliedPost);
    return appliedPre != appliedPost;    // Returns true if there's been a change in visualisation application state
}   // end toggleVis


bool ActionVisualise::doAction( FVS& fvs, const QPoint& mc)
{
    setViewsToProcess( fvs);
    std::for_each( std::begin(fvs), std::end(fvs), [&](FV* f){ toggleVis(f, &mc);});
    return !fvs.empty();
}   // end doAction


void ActionVisualise::purge( const FM* fm)
{
    const FVS& fvs = fm->fvs();
    // Don't remove the visualisation (e.g. fv->remove(_vis)), because this will remove the layer
    // and purging is done as part of a reset operation so visualisation layers should remain so
    // they can be reapplied in FaceView::reset after rebuilding of the actor.
    std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ _vis->purge(fv);});
    _vis->purge(fm);
}   // end purge


void ActionVisualise::clean( const FM* fm)
{
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ fv->remove(_vis);});    // Removing the visualisation layer okay here.
    std::for_each( std::begin(fvs), std::end(fvs), [=](FV* fv){ _vis->purge(fv);});
    _vis->purge(fm);
}   // end clean


// protected
size_t ActionVisualise::setViewsToProcess( FVS& fvs) const
{
    if ( _vis->applyToSelectedModel())
        fvs.includeModelViews();
    if ( _vis->applyToAllInViewer())
        fvs.includeViewerViews();
    if ( _vis->applyToSelectedModel())
        fvs.includeModelViews();
    return fvs.size();
}   // end setViewsToProcess
