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

#include <FaceActionManager.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <PluginsLoader.h>  // QTools
#include <QApplication>
#include <QString>
#include <algorithm>
#include <sstream>
#include <cassert>
#include <iomanip>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::FileIO::FaceModelManager;
using FaceTools::FileIO::LoadFaceModelsHelper;
using FaceTools::Interactor::MVI;
using FaceTools::Interactor::ViewerInteractionManager;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

// public
FaceActionManager::FaceActionManager( FaceModelViewer* viewer, size_t llimit, QWidget* parent)
    : QObject(), _pdialog( new QTools::PluginsDialog( parent)),
        _fmm( new FaceModelManager( parent, llimit)),
        _interactions( new ViewerInteractionManager( viewer)),
        _selector(viewer)
{
    qRegisterMetaType<FaceTools::Action::ChangeEventSet>("FaceTools::Action::ChangeEventSet");
    qRegisterMetaType<FaceTools::Action::ChangeEvent>("FaceTools::Action::ChangeEvent");
    qRegisterMetaType<FaceTools::FaceControlSet>("FaceTools::FaceControlSet");

    //connect( _interactions, &ViewerInteractionManager::onActivatedViewer, &_selector, &ModelSelector::doSwitchSelectedToViewer);
    connect( &_selector, &ModelSelector::onSelected, [this]( FaceControl* fc, bool v){ setReady(fc,v);});
    _vman.makeDefault(this);
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
        //std::cerr << "[INFO] FaceTools::FaceActionManager::addAction: " << fa->debugActionName() << " @ " << fa << std::endl;
        _actions.insert(fa);
        connect( fa, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
        connect( fa, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);

        _vman.add(fa);  // Does nothing if not a visualisation action
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
void FaceActionManager::doOnChangedData( FaceControl *fc)
{
    fc->data()->setSaved(false);
    setReady( fc, true);    // Already selected
    emit onUpdateSelected( fc);
}   // end doOnChangedData


// private slot
void FaceActionManager::doOnActionStarting()
{
    // Disable user actions upon an action starting.
    std::for_each(std::begin(_actions), std::end(_actions), [](auto a){ a->setEnabled(false);});
}   // end doOnActionStarting


// private slot
void FaceActionManager::doOnActionFinished( ChangeEventSet cset, FaceControlSet workSet, bool)
{
    _mutex.lock();
    FaceAction* sact = qobject_cast<FaceAction*>(sender());
    assert(sact);
    /*
    std::cerr << "[INFO] FaceTools::Action::FaceActionManager::doOnActionFinished: "
              << sact->debugActionName() << " (" << sact << ") "
              << cset.size() << " CHANGE EVENTS" << std::endl;
    */

    FaceViewerSet vwrs = workSet.viewers();    // The viewers before processing (e.g. before load or close)

    if ( !cset.empty())
    {
        processFinishedAction( sact, &cset, &workSet);  // Process after action bits
        FaceViewerSet vwrs1 = workSet.viewers();        // The viewers after processing (e.g. after load or close)
        vwrs.insert(vwrs1.begin(), vwrs1.end());

        // If responding to LOADED_MODEL, each just loaded model now has a single FaceControl inside workSet,
        // and cset now contains both LOADED_MODEL and GEOMETRY_CHANGE.

        // Push actions to execute to the back of the cue (if any)
        if ( !cset.empty())
        {
            _actions.erase(sact);   // Don't push the just completed action!
            std::for_each( std::begin(_actions), std::end(_actions), [&](auto act){ _aqueue.pushIfShould( act, &cset);});
            _actions.insert(sact);
        }   // end if
    }   // end if

    // Visualisations will have been reapplied unless purging their data made them
    // unavailable in which case these views need default visualisations setting.
    _vman.enforceVisualisationConformance( &workSet);
    std::for_each( std::begin(vwrs), std::end(vwrs), [](auto v){ v->updateRender();});

    bool pflag = false;
    FaceAction* nact = _aqueue.popOrClear( workSet, pflag); // Prep the queued action (if any)
 
    _mutex.unlock();

    if ( nact)  // Execute the follow-on action (if any) - applied to every member of the worked on set.
    {
        //std::cerr << " ++ Starting " << nact->debugActionName() << " with process flag " << std::boolalpha << pflag << std::endl;
        nact->process( pflag);
    }   // end if
    else
    {
        // Update the ready state for all actions using the currently selected FaceControl (not the worked on set).
        FaceControl* sel = _selector.selected();
        std::for_each(std::begin(_actions), std::end(_actions), [&](auto a){ a->setReady( sel, true);});
        emit onUpdateSelected( sel);
    }   // end else
}   // end doOnActionFinished


// private
void FaceActionManager::processFinishedAction( FaceAction* sact, ChangeEventSet *cset, FaceControlSet *workSet)
{
    if ( cset->count(CLOSE_MODEL) > 0)
    {
        //std::cerr << "[INFO] FaceTools::Action::FaceActionManager::doOnActionFinished: closing model(s)" << std::endl;
        const FaceModelSet& fms = workSet->models(); // Copy out the models
        std::for_each( std::begin(fms), std::end(fms), [this](auto fm){ this->close(fm);});
        cset->erase(CLOSE_MODEL);   // Close model done
        workSet->clear();
    }   // end if
    else if ( cset->count(LOADED_MODEL) > 0)
    {
        // Obtain the FaceControl's for the just loaded FaceModels
        assert(workSet->empty());
        for ( const std::string& fpath : _fmm->loader()->lastLoaded())
        {
            FaceModel* fm = _fmm->model(fpath);
            assert(fm);
            workSet->insert( _selector.addFaceControl(fm));   // Causes ModelSelector::onSelected to be fired
        }   // end for
        cset->insert(GEOMETRY_CHANGE);
    }   // end if
    else if ( !cset->empty())   // Cause actions to respond to change events
    {
        const FaceModelSet& fms = workSet->models(); // The affected models
        // Purge actions due to received change events.
        _actions.erase(sact);    // Don't purge from the sending action!
        std::for_each( std::begin(_actions), std::end(_actions), [&]( auto a){ this->testPurge( a, cset, &fms);});
        _actions.insert(sact);

        // Geometry change events necessitate rebuilding the view models over all affected FaceControls.
        if ( cset->count(GEOMETRY_CHANGE) > 0)
        {
            for ( FaceModel* fm : fms)
            {
                const FaceControlSet& fcs = fm->faceControls();
                std::for_each( std::begin(fcs), std::end(fcs), [](auto fc){ fc->view()->reset();});
            }   // end for
        }   // end if
    }   // end else if
}   // end processFinishedAction


// purge action with fset if the intersection of action's purgeEvents() with cset is non-empty.
void FaceActionManager::testPurge( FaceAction* act, const ChangeEventSet* cset, const FaceModelSet* fms)
{
    const ChangeEventSet* s = cset;
    const ChangeEventSet* t = &act->purgeEvents();
    if ( t->size() < s->size())
        std::swap(s,t);

    for ( auto c : *s)
    {
        if ( t->count(c) > 0)
        {
            std::for_each( std::begin(*fms), std::end(*fms), [=](auto fm){ act->purge(fm);});
            return;
        }   // end if
    }   // end for
}   // end testPurge


// private
void FaceActionManager::setReady( FaceControl* fc, bool v)
{
    std::for_each(std::begin(_actions), std::end(_actions), [=](auto a){ a->setReady(fc,v);});
    fc->data()->updateRenderers();
    emit onUpdateSelected( fc);
}   // end setReady


// private
void FaceActionManager::close( FaceModel* fm)
{
    std::for_each(std::begin(_actions), std::end(_actions), [=](auto a){ a->clean(fm);});
    _selector.remove(fm);
    _fmm->close(fm);

    // Even though ModelSelector::remove results in a call to setReady, this comes BEFORE the FaceModel
    // itself is closed. Some actions may depend upon this state update on FaceModelManager, so need to
    // check the enabled state of the actions again.
    std::for_each(std::begin(_actions), std::end(_actions), [&](auto a){ a->testSetEnabled();});
}   // end close

