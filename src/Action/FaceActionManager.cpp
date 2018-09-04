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
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <PluginsLoader.h>  // QTools
#include <QApplication>
#include <algorithm>
#include <sstream>
#include <cassert>
#include <iomanip>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventSet;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Interactor::MVI;
using FaceTools::Interactor::ViewerInteractionManager;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FVS;
using FaceTools::FM;

// public
FaceActionManager::FaceActionManager( FMV* viewer, size_t llimit, QWidget* parent)
    : QObject(), _pdialog( new QTools::PluginsDialog( parent)),
        _fmm( new FaceModelManager( parent, llimit)),
        _interactions( new ViewerInteractionManager( viewer)),
        _vman(nullptr),
        _selector(viewer)
{
    //connect( _interactions, &ViewerInteractionManager::onActivatedViewer, &_selector, &ModelSelector::doSwitchSelectedToViewer);
    connect( &_selector, &ModelSelector::onSelected, [this]( FV* fv, bool v){ setReady(fv,v);});
    _vman = new VisualisationsManager;
    _vman->init(this);
}   // end ctor


// public
FaceActionManager::~FaceActionManager()
{
    // We delete the actions in this manner because there's a chance that an action
    // within its destructor could do something weird like calling process.
    while ( _actions.empty())
    {
        FaceAction* fa = *_actions.begin();
        fa->disconnect(this);
        _actions.erase(fa);
        delete fa;
    }   // end while
    delete _vman;
    delete _interactions;
    delete _fmm;
    delete _pdialog;
}   // end dtor


// public
void FaceActionManager::loadPlugins()
{
    const QString dllsDir = QApplication::applicationDirPath() + "/plugins";
    QTools::PluginsLoader ploader( dllsDir.toStdString());
    std::cerr << "Plugins directory: " << ploader.getPluginsDir().absolutePath().toStdString() << std::endl;
    connect( &ploader, &QTools::PluginsLoader::loadedPlugin, this, &FaceActionManager::addPlugin);
    ploader.loadPlugins();
    _pdialog->addPlugins( ploader);
}   // end loadPlugins


// private slot
void FaceActionManager::addPlugin( QTools::PluginInterface* plugin)
{
    FaceAction* faction = nullptr;
    FaceActionGroup* grp = qobject_cast<FaceActionGroup*>(plugin);
    if ( grp)
    {
        for ( const QString& iid : grp->getInterfaceIds())
        {
            faction = qobject_cast<FaceAction*>( grp->getInterface(iid));
            addAction(faction);
        }   // end for
        emit addedActionGroup( grp);
    }   // end if
    else if ( faction = qobject_cast<FaceAction*>(plugin))
    {
        addAction( faction);
        emit addedAction( faction);
    }   // end if
}   // end addPlugin


// public
QAction* FaceActionManager::addAction( FaceAction* fa)
{
    if ( _actions.count(fa) == 0)
    {
        //std::cerr << "[INFO] FaceTools::FaceActionManager::addAction: " << fa->dname() << " @ " << fa << std::endl;
        _actions.insert(fa);
        connect( fa, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
        connect( fa, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);

        _vman->add(fa);  // Does nothing if not a visualisation action
        fa->init();

        MVI* interactor = fa->interactor();
        if ( interactor) // Set the default viewer for the action's interactor if it defines one.
        {
            interactor->setViewer( _selector.interactor()->viewer());
            connect( interactor, &MVI::onChangedData, this, &FaceActionManager::doOnChangedData);
        }   // end if
    }   // end if
    return fa->qaction();
}   // end addAction


// private slot
void FaceActionManager::doOnChangedData( FV* fv)
{
    fv->data()->setSaved(false);
    setReady( fv, true);    // Already selected so fine to do
    emit onUpdateSelected( fv);
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
    std::for_each(std::begin(_actions), std::end(_actions), [](auto a){ a->setEnabled(false);});
    _mutex.unlock();
}   // end doOnActionStarting


// private slot
void FaceActionManager::doOnActionFinished( EventSet evs, FVS workSet, bool)
{
    _mutex.lock();
    FaceAction* sact = qobject_cast<FaceAction*>(sender());
    printActionInfo( sact, &evs);

    FMVS vwrs = workSet.viewers();    // The viewers before processing (e.g. before load or close)
    if ( !evs.empty())
    {
        processFinishedAction( sact, evs, workSet);  // Process after action bits
        FMVS vwrs1 = workSet.viewers();     // The viewers after processing (e.g. after load or close)
        vwrs.insert(vwrs1.begin(), vwrs1.end());
        // If responding to LOADED_MODEL, each just loaded model now has a single FaceView inside workSet,
        // and evs now contains both LOADED_MODEL and GEOMETRY_CHANGE.
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
        FV* sel = _selector.selected();
        std::for_each(std::begin(_actions), std::end(_actions), [=](auto a){ a->resetReady( sel);});
        emit onUpdateSelected( sel);
        _mutex.unlock();
    }   // end else

    std::for_each( std::begin(vwrs), std::end(vwrs), [](auto v){ v->updateRender();});
}   // end doOnActionFinished


// private
void FaceActionManager::processFinishedAction( FaceAction* sact, EventSet &evs, FVS &workSet)
{
    if ( evs.count(CLOSE_MODEL) > 0)
    {
        const FMS& fms = workSet.models(); // Copy out the models
        std::for_each( std::begin(fms), std::end(fms), [this](auto fm){ this->close(fm);});
        evs.erase(CLOSE_MODEL);   // Close model done
        workSet.clear();
    }   // end if
    else if ( evs.count(LOADED_MODEL) > 0)
    {
        // Obtain the FaceView's for the just loaded FaceModels
        workSet.clear();
        for ( const std::string& fpath : _fmm->loader()->lastLoaded())
        {
            FM* fm = _fmm->model(fpath);
            assert(fm);
            FV* nfv = _selector.addFaceView(fm);
            _selector.setSelected( nfv, true);
            workSet.insert( nfv);
        }   // end for
        evs.insert(GEOMETRY_CHANGE);
    }   // end if
    else if ( !evs.empty())   // Cause actions to respond to change events
    {
        FMS fms = workSet.models();   // Copy out

        // Purge actions due to received change events (not the sending action)
        _actions.erase(sact);
        for ( FaceAction* act : _actions)
        {
            if ( std::any_of(std::begin(evs), std::end(evs), [=](auto ev){ return act->isPurgeEvent(ev);}))
                std::for_each( std::begin(fms), std::end(fms), [=](auto fm){ act->purge(fm);});
        }   // end for
        _actions.insert(sact);

        // Geometry change events require rebuilding the actors associated with the affected FaceModels.
        // This means that workSet must be changed to hold all views of the affected models.
        if ( evs.count(GEOMETRY_CHANGE) > 0)
        {
            workSet.clear();
            std::for_each( std::begin(fms), std::end(fms), [&](const FM* fm){ workSet.insert(fm);});
            std::for_each( std::begin(workSet), std::end(workSet), [](FV* fv){ fv->reset();});
        }   // end if
    }   // end else if

    _actions.erase(sact);   // Push candidate follow-on actions to the back of the execution queue (not the sending action).
    std::for_each( std::begin(_actions), std::end(_actions), [&](FaceAction* act){ _aqueue.push(act, workSet, evs);});
    _actions.insert(sact);
}   // end processFinishedAction


// private
void FaceActionManager::setReady( FV* fv, bool v)
{
    std::for_each(std::begin(_actions), std::end(_actions), [=](auto a){ a->setReady(fv,v);});
    fv->data()->updateRenderers();
    emit onUpdateSelected( fv);
}   // end setReady


// private
void FaceActionManager::close( FM* fm)
{
    std::cerr << "[INFO] FaceTools::Action::FaceActionManager::close: closing model " << fm << std::endl;
    fm->lockForWrite();
    //const FVS& fvs = fm->fvs();
    for ( FaceAction* a : _actions)
    {
        a->purge(fm);
        a->clean(fm);
    }   // end for
    fm->unlock();
    _selector.remove(fm);
    _fmm->close(fm);

    // Even though ModelSelector::remove results in a call to setReady, this comes BEFORE the FaceModel
    // itself is closed. Some actions may depend upon this state update on FaceModelManager, so need to
    // check the enabled state of the actions again.
    std::for_each(std::begin(_actions), std::end(_actions), [&](auto a){ a->testSetEnabled();});
}   // end close

