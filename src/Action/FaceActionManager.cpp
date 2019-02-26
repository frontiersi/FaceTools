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

#include <FaceActionManager.h>
#include <FaceModelManager.h>
#include <ActionVisualise.h>
#include <BoundingVisualisation.h>
#include <FaceModelViewer.h>
#include <ModelSelector.h>
#include <U3DCache.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <QApplication>
#include <algorithm>
#include <sstream>
#include <cassert>
#include <iomanip>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventSet;
using FaceTools::FileIO::FMM;
using FaceTools::Interactor::MVI;
using FaceTools::Interactor::ViewerInteractionManager;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FVS;
using FaceTools::FM;

// public
FaceActionManager::FaceActionManager( FMV* viewer)
    : QObject(), _interactions( new ViewerInteractionManager( viewer))
{
    ModelSelector::create( this, viewer);
    addAction( new ActionVisualise( &_bvis));
}   // end ctor


void FaceActionManager::addViewer( FMV* v){ _interactions->addViewer(v);}


// public
FaceActionManager::~FaceActionManager()
{
    // We delete the actions in this manner because there's a chance that an action
    // within its destructor could do something weird like calling process.
    while ( !_actions.empty())
    {
        FaceAction* fa = *_actions.begin();
        fa->disconnect(this);
        _actions.erase(fa);
        delete fa;
    }   // end while
    delete _interactions;
}   // end dtor


// public
QAction* FaceActionManager::addAction( FaceAction* fa)
{
    if ( _actions.count(fa) == 0)
    {
        _actions.insert(fa);
        connect( fa, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
        connect( fa, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);

        fa->init();

        MVI* interactor = fa->interactor();
        if ( interactor) // Set the default viewer for the action's interactor if it defines one.
        {
            interactor->setViewer( ModelSelector::viewer());
            connect( interactor, &MVI::onChangedData, this, &FaceActionManager::doOnChangedData);
        }   // end if

        emit addedAction(fa);
    }   // end if
    return fa->qaction();
}   // end addAction


// private slot
void FaceActionManager::doOnChangedData( FV* fv)
{
    fv->data()->setSaved(false);
    doOnSelected( fv, true);    // Already selected so fine to do
}   // end doOnChangedData


namespace {
void printActionInfo( const FaceAction* act, const EventSet* evs=nullptr)
{
    assert(act);
    const std::string estr = evs ? "  FINISH " : "   START ";
    std::cerr << estr << act->dname() << " <" << act << ">";
    if ( act->isCheckable())
        std::cerr << (act->isChecked() ? " {ON}" : " {OFF}");
    if ( evs && !evs->empty())
        std::cerr << " (" << evs->size() << " event" << (evs->size() > 1 ? "s" : "") << ")";
    std::cerr << std::endl;
}   // end printActionInfo
}   // end namespace


// private slot
void FaceActionManager::doOnActionStarting()
{
    _mutex.lock();
    printActionInfo( qobject_cast<FaceAction*>(sender()));

    // Disable actions upon an action starting.
    // TODO Only disable actions that need to address the same data as the action that's just started.
    std::for_each(std::begin(_actions), std::end(_actions), [](FaceAction* a){ a->setEnabled(false);});
    _mutex.unlock();
}   // end doOnActionStarting


// private slot
void FaceActionManager::doOnActionFinished( EventSet evs, FVS workSet, bool)
{
    _mutex.lock();
    FaceAction* sact = qobject_cast<FaceAction*>(sender());
    printActionInfo( sact, &evs);

    FMVS vwrs = workSet.dviewers();    // The viewers before processing
    if ( !evs.empty())
    {
        processFinishedAction( sact, evs, workSet);  // Process after action bits
        FMVS vwrs1 = workSet.dviewers();     // The viewers after processing
        vwrs.insert(vwrs1.begin(), vwrs1.end());
    }   // end if

    bool pflag = false;
    FaceAction* nact = _aqueue.pop( pflag); // Prep the queued action (if any)
 
    if ( nact)  // Execute the follow-on action (if any) - applied to every member of the worked on set.
    {
        _mutex.unlock();
        nact->process( pflag);
    }   // end if
    else
    {
        // Update the ready state for all actions using the currently selected FaceView (not the worked on set).
        FV* sel = ModelSelector::selected(); // May be null!
        std::for_each(std::begin(_actions), std::end(_actions), [=](FaceAction* a){ a->resetReady( sel);});
        emit onUpdateSelected( sel ? sel->data() : nullptr, sel);       // Ensure clients know to update status of selected post-action
        _mutex.unlock();
    }   // end else

    std::for_each( std::begin(vwrs), std::end(vwrs), [](FMV* v){ v->updateRender();});
}   // end doOnActionFinished


// private
void FaceActionManager::processFinishedAction( FaceAction* sact, EventSet &evs, FVS &workSet)
{
    FMS fms = workSet.models();   // Copy out

    if ( evs.count(LOADED_MODEL))
    {
        evs.insert(GEOMETRY_CHANGE);
        evs.insert(VIEWER_CHANGE);
        evs.insert(VIEW_CHANGE);
    }   // end if
    else if ( evs.count(CLOSE_MODEL) > 0)
    {
        FMVS avwrs;
        for ( FM* fm : fms)
        {
            // Consolidate set of viewers within which this model's views are shown for after close.
            FMVS vwrs = fm->fvs().dviewers();
            avwrs.insert( std::begin(vwrs), std::end(vwrs));
            qInfo( "Closing model %p", static_cast<void*>(fm));
            close(fm);  // Close the FaceModel and remove its views.
        }   // end for

        // Need to notify other FaceViews in the affected viewers of VIEWER_CHANGE event
        evs.insert(VIEWER_CHANGE);
        workSet.clear();
        for ( FMV* fmv : avwrs)
            workSet.insert(fmv->attached());
    }   // end if
    else if ( !evs.empty())   // Cause actions to respond to change events
    {
        // Purge actions due to received change events (not the sending action)
        _actions.erase(sact);
        for ( FaceAction* act : _actions)
        {
            if ( std::any_of(std::begin(evs), std::end(evs), [=](EventId ev){ return act->isPurgeEvent(ev);}))
                std::for_each( std::begin(fms), std::end(fms), [=](FM* fm){ act->purge(fm);});
        }   // end for
        _actions.insert(sact);

        // Geometry change events require rebuilding the actors associated with the affected FaceModels.
        // This means that workSet must be changed to hold all views of the affected models.
        if ( evs.count(GEOMETRY_CHANGE) > 0)
        {
            workSet.clear();
            for ( const FM* fm : fms)
            {
                workSet.insert(fm);
                U3DCache::purge(fm);    // The U3D model will need to be updated too!
            }   // end for
            std::for_each( std::begin(workSet), std::end(workSet), [](FV* fv){ fv->reset();});
        }   // end if
    }   // end else if

    _actions.erase(sact);   // Push candidate follow-on actions to the back of the execution queue (not the sending action).
    std::for_each( std::begin(_actions), std::end(_actions), [&](FaceAction* act){ _aqueue.push(act, workSet, evs);});
    _actions.insert(sact);
}   // end processFinishedAction


void FaceActionManager::doOnSelected( FV* fv, bool v)
{
    for ( FaceAction* act : _actions)
        act->setReady(fv, v);

    fv->data()->updateRenderers();
    emit onUpdateSelected( fv->data(), v);
}   // end setReady


// public
void FaceActionManager::close( FM* fm)
{
    fm->lockForWrite();
    for ( FaceAction* a : _actions)
    {
        a->purge(fm);
        a->clean(fm);
    }   // end for
    fm->unlock();
    ModelSelector::remove(fm);
    FMM::close(fm);

    // Even though ModelSelector::remove results in a call to setReady, this comes BEFORE the FaceModel
    // itself is closed. Some actions may depend upon this state update on FaceModelManager, so need to
    // check the enabled state of the actions again.
    std::for_each(std::begin(_actions), std::end(_actions), [&](FaceAction* a){ a->testSetEnabled();});
}   // end close

