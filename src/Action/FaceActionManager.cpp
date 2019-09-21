/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <Action/FaceActionManager.h>
#include <FileIO/FaceModelManager.h>
#include <FaceModelViewer.h>
#include <Action/ActionMoveView.h>
#include <Action/ModelSelector.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <cassert>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventGroup;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


namespace {

void syncViewActorsToModelTransform( const FM* fm)
{
    for ( FaceTools::Vis::FV* fv : fm->fvs())
        fv->syncToModelTransform();
}   // end syncViewActorsToModelTransform


// Rebuild the actors associated with the model.
void rebuildViewActors( const FM* fm)
{
    for ( FaceTools::Vis::FV* fv : fm->fvs())
        fv->reset();
}   // end rebuildViewActors

}   // end namespace


FaceActionManager::Ptr FaceActionManager::_singleton;


// static
FaceActionManager::Ptr FaceActionManager::get( QWidget* parent)
{
    if ( !_singleton)
    {
        _singleton = Ptr(new FaceActionManager, [](FaceActionManager* fam){ delete fam;});
        _singleton->_parent = parent;
    }   // end if
    return _singleton;
}   // end get


// private
FaceActionManager::FaceActionManager()
{
    using FaceTools::Interactor::SelectNotifier;
    const SelectNotifier* sn = MS::selector();
    connect( sn, &SelectNotifier::onSelected, [this](){ this->doEvent( Event::MODEL_SELECT);});
}   // end ctor


// static
QAction* FaceActionManager::registerAction( FaceAction* act)
{
    auto& acts = get()->_actions;
    assert( acts.count(act) == 0);
    connect( act, &FaceAction::onEvent, &*get(), &FaceActionManager::doEvent);
    connect( act, &FaceAction::onShowHelp, [](const QString& tok){ emit get()->onShowHelp(tok);});
    acts.insert(act);
    act->init(_singleton->_parent);
    act->refreshState();
    emit _singleton->onRegisteredAction(act);
    return act->qaction();
}   // end registerAction


// static
void FaceActionManager::close( const FM* fm)
{
    assert(fm);
    fm->lockForRead();
    const auto& acts = get()->_actions;
    for ( FaceAction* act : acts)
        act->purge(fm, Event::CLOSED_MODEL);
    fm->unlock();
    MS::remove(fm);
    FileIO::FMM::close(fm);

    // If there are other FaceModels loaded, select one with a preference for models from the default viewer.
    if ( FileIO::FMM::numOpen() > 0)
    {
        bool movedView = false;
        Vis::FV *nextfv = nullptr;

        if ( !MS::defaultViewer()->attached().empty())
            nextfv = MS::defaultViewer()->attached().first();
        else
        {
            // Get the next FaceView to select from one of the other viewers.
            for ( const FMV* fmv : MS::viewers())
            {
                if ( !fmv->attached().empty())
                {
                    nextfv = fmv->attached().first();
                    break;
                }   // end if
            }   // end for

            // Since the default viewer is empty, we move this newly selected FaceView into the default viewer.
            ActionMoveView::move( nextfv, MS::defaultViewer());
            movedView = true;
        }   // end else

        MS::setSelected( nextfv);
        if ( movedView)
            get()->doEvent( Event::VIEWER_CHANGE);
    }   // end if
}   // end close


void FaceActionManager::doEvent( EventGroup egrp)
{
    const Event E = egrp.event();
    const FM* fm = MS::selectedModel();
    if ( egrp.is(Event::ACT_CANCELLED))
        return;

#ifndef NDEBUG
    std::cerr << "Event " << egrp.name() << " | selected model = " << std::hex << fm << std::dec << std::endl;
#endif

    // The sending action should not be triggered by its own events (note that
    // executed actions always refresh their own state upon completion).
    // NOTE sact may be null since a FaceAction may not be causing this call!
    FaceAction* sact = qobject_cast<FaceAction*>( sender());

    // Purge actions first.
    for ( FaceAction* act : _actions)
    {
        if ( act != sact && act->isPurgeEvent(E))
            act->purge(fm, E);
    }   // end for

    // Geometry change events require rebuilding the actors associated with the affected FaceModels.
    // This also reapplies visualisations and it is necessary that any visualisations that rely upon
    // associations of data be fully purged first which is why the above loop to purge the actions
    // comes before this check.
    if ( fm && (egrp.has(Event::GEOMETRY_CHANGE) || egrp.has(Event::ORIENTATION_CHANGE)))
    {
        if ( !egrp.has( Event::ALL_VIEWS))
            rebuildViewActors( fm);
        else
        {
            const FVS& aset = MS::selectedView()->viewer()->attached();
            for ( const FM* vm : aset.models())
                rebuildViewActors(vm);
        }   // end else
    }   // end if

    // Synchronise affine transformations across view actors
    if ( fm && egrp.has( Event::AFFINE_CHANGE))
    {
        if ( !egrp.has( Event::ALL_VIEWS))
            syncViewActorsToModelTransform( fm);
        else
        {
            const FVS& aset = MS::selectedView()->viewer()->attached();
            for ( const FM* vm : aset.models())
                syncViewActorsToModelTransform( vm);
        }   // end else
    }   // end if

    if ( egrp.has(Event::LANDMARKS_CHANGE) || egrp.has(Event::FACE_DETECTED) || egrp.has(Event::ASSESSMENT_CHANGE))
    {
        if ( !egrp.has( Event::ALL_VIEWS))
            MS::syncBoundingVisualisation(fm);
        else
        {
            const FVS& aset = MS::selectedView()->viewer()->attached();
            for ( const FM* vm : aset.models())
                MS::syncBoundingVisualisation(vm);
        }   // end else
    }   // end if

    // If the action did anything to the camera, always set the interaction
    // mode back to camera interaction mode.
    if ( egrp.has(Event::CAMERA_CHANGE))
        MS::setInteractionMode(IMode::CAMERA_INTERACTION);

    // Have actions recheck their own state and whether or not they're enabled, then see
    // if the received event triggers them.
    for ( FaceAction* act : _actions)
    {
        // Sending actions should not be able to trigger themselves, and
        // actions are not reentrant generally speaking so running actions are also ignored.
        if ( act == sact || (act->isRunning() && !act->isReentrant()))
            continue;

        // In refreshing state, actions decide whether to enable themselves and check themselves.
        act->refreshState(E);

        if ( act->isEnabled())
        {
            if ( act->isTriggerEvent(E))
                act->execute(E);
        }   // end if
    }   // end for

    // Finally notify the client application of updates and re-render.
    emit onUpdate( fm);
    MS::updateRender();
}   // end doEvent
