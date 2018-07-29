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
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

// public
FaceActionManager::FaceActionManager( FaceModelViewer* viewer, size_t llimit, QWidget* parent)
    : QObject(), _pdialog( new QTools::PluginsDialog( parent)),
        _fmm( new FaceModelManager( parent, llimit)), _selector(viewer)
{
    qRegisterMetaType<FaceTools::Action::ChangeEventSet>("FaceTools::Action::ChangeEventSet");
    qRegisterMetaType<FaceTools::Action::ChangeEvent>("FaceTools::Action::ChangeEvent");
    qRegisterMetaType<FaceTools::FaceControlSet>("FaceTools::FaceControlSet");

    connect( &_selector, &ModelSelector::onSelected, [this]( FaceControl* fc, bool v){ setReady(fc,v);});
    _vman.makeDefault(this);    // Make the default visualisations
}   // end ctor


// public
FaceActionManager::~FaceActionManager()
{
    std::for_each( std::begin(_actions), std::end(_actions), [](auto a){ delete a;});
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
QAction* FaceActionManager::addAction( FaceAction* faction)
{
    assert( faction);
    if ( _actions.count(faction) == 0)
    {
        faction->init();
        _actions.insert(faction);
        connect( faction, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
        connect( faction, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);
        _vman.add(faction); // VisualisationManager won't add the action if it doesn't cast to ActionVisualise

        MVI* interactor = faction->interactor();
        if ( interactor) // Set the default viewer for the action's interactor if it defines one.
        {
            interactor->setViewer( _selector.interactor()->viewer());
            connect( interactor, &MVI::onChangedData, this, &FaceActionManager::doOnChangedData);
        }   // end if
    }   // end if
    return faction->qaction();
}   // end addAction


// private slot
void FaceActionManager::doOnChangedData( const FaceControl *fc)
{
    fc->data()->setSaved(false);
    setReady( const_cast<FaceControl*>(fc), true);    // Already selected
    emit onUpdateSelected();
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
    std::cerr << "[INFO] FaceTools::Action::FaceActionManager::doOnActionFinished: "
              << sact->debugActionName() << " (" << sact << ") "
              << cset.size() << " CHANGE EVENTS" << std::endl;

    FaceViewerSet vwrs = workSet.viewers();    // The viewers before processing (e.g. before load or close)

    if ( !cset.empty())
    {
        processFinishedAction( sact, &cset, &workSet);  // Process after action bits
        FaceViewerSet vwrs1 = workSet.viewers();        // The viewers after processing (e.g. after load or close)
        vwrs.insert(vwrs1.begin(), vwrs1.end());

        // Push actions to execute to the back of the cue (if any)
        if ( !cset.empty())
        {
            _actions.erase(sact);
            for ( FaceAction* act : _actions)
                _aqueue.testPush( act, &cset);
            _actions.insert(sact);
        }   // end if
    }   // end if

    // Visualisations will have been reapplied unless purging their data made them
    // unavailable in which case these views need default visualisations setting.
    _vman.enforceVisualisationConformance( &workSet);
    std::for_each( std::begin(vwrs), std::end(vwrs), [](auto v){ v->updateRender();});

    std::cerr << "[INFO] FaceTools::Action::FaceActionManager::doOnActionFinished: " << _aqueue.size() << " actions left on queue" << std::endl;

    bool pflag = false;
    FaceAction* nact = _aqueue.pop( pflag);
    FaceControlSet sel = _selector.selected();

    while ( nact)
    {
        nact->resetReady( sel);
        if ( nact->testEnabled())   // Found next action to execute
            break;
        else
        {
            std::cerr << " Skipping queued action " << nact->debugActionName() << " (unavailable to selected models)" << std::endl;
            nact = _aqueue.pop( pflag); // Get next action on the queue (returns null if no more)
        }   // end else
    }   // end if

    // Update the ready state for the actions
    std::for_each(std::begin(_actions), std::end(_actions), [&](auto a){ a->resetReady(sel);});

    emit onUpdateSelected();

    _mutex.unlock();

    // Finally, execute the next queued action (if any)
    if ( nact)
    {
        std::cerr << " ++ Starting " << nact->debugActionName() << " with process flag " << std::boolalpha << pflag << std::endl;
        nact->process( pflag);
    }   // end if
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
    else
    {
        if ( cset->count(LOADED_MODEL) > 0)
        {
            assert(workSet->empty());
            doLoadedModels( workSet);  // Obtain the FaceControl's for the just loaded FaceModels
            cset->insert(GEOMETRY_CHANGE);
        }   // end if
        else if ( !cset->empty())   // Cause actions to respond to change events
        {
            const FaceModelSet& fms = workSet->models(); // The affected models
            _actions.erase(sact);    // Don't purge from the sender!
            // Purge actions due to received change events.
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
    }   // end else
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
    emit onUpdateSelected();
}   // end setReady


// private
void FaceActionManager::close( FaceModel* fm)
{
    std::for_each(std::begin(_actions), std::end(_actions), [=](auto a){ a->purge(fm);});
    _selector.remove(fm);
    _fmm->close(fm);

    // Even though ModelSelector::remove results in a call to setReady, this comes BEFORE the FaceModel
    // itself is closed. Some actions may depend upon this state update on FaceModelManager, so need to
    // check the enabled state of the actions again.
    std::for_each(std::begin(_actions), std::end(_actions), [](auto a){ a->testSetEnabled();});
}   // end close


// private
void FaceActionManager::doLoadedModels( FaceControlSet* fcs)
{
    for ( const std::string& fpath : _fmm->loader()->lastLoaded())
    {
        FaceModel* fm = _fmm->model(fpath);
        assert(fm);
        fcs->insert( _selector.addFaceControl(fm));   // Will cause ModelSelector::onSelected to be fired
    }   // end for
}   // end doLoadedModels
