/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
#include <Metric/MetricManager.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <functional>
#include <cassert>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::Action::ModelSelector;
using MM = FaceTools::Metric::MetricManager;
using FaceTools::FM;


namespace {

void pokeViewTransformsWithModel( const FM* fm)
{
    vtkSmartPointer<vtkMatrix4x4> vmat = r3dvis::toVTK( fm->transformMatrix());
    for ( FaceTools::Vis::FV* fv : fm->fvs())
        fv->pokeTransform( vmat);
}   // end pokeViewTransformsWithModel


// Rebuild the actors associated with the model.
void rebuildViewActors( const FM* fm)
{
    for ( FaceTools::Vis::FV* fv : fm->fvs())
        fv->reset();
}   // end rebuildViewActors


void applyFnToModels( const Event &e, const FM* fm, const std::function<void( const FM*)> &fn)
{
    if ( !has( e, Event::ALL_VIEWS))
        fn( fm);
    else
    {
        const FaceTools::FVS& aset = MS::selectedView()->viewer()->attached();
        for ( const FM* vm : aset.models())
            fn( vm);
    }   // end else
}   // end applyFnToModels

}   // end namespace


FaceActionManager::Ptr FaceActionManager::s_singleton;
QMutex FaceActionManager::s_closeLock;


// static
FaceActionManager::Ptr FaceActionManager::get( QWidget* parent)
{
    if ( !s_singleton)
    {
        s_singleton = Ptr(new FaceActionManager, [](FaceActionManager* fam){ delete fam;});
        s_singleton->_parent = parent;
    }   // end if
    return s_singleton;
}   // end get


// private
FaceActionManager::FaceActionManager()
{
    const Interactor::SelectNotifier *sn = MS::selector();
    connect( sn, &Interactor::SelectNotifier::onSelected, [this](){ this->doEvent( Event::MODEL_SELECT);});
}   // end ctor


// static
QAction* FaceActionManager::registerAction( FaceAction* act)
{
    auto& acts = get()->_actions;
    assert( acts.count(act) == 0);
    connect( act, &FaceAction::onEvent, &*get(), &FaceActionManager::doEvent);
    connect( act, &FaceAction::onShowHelp, [](const QString& tok){ emit get()->onShowHelp(tok);});
    acts.insert(act);
    act->_init(s_singleton->_parent);
    act->refreshState();
    emit s_singleton->onRegisteredAction(act);
    return act->qaction();
}   // end registerAction


// static
void FaceActionManager::close( const FM* fm)
{
    assert(fm);
    s_closeLock.lock();
    fm->lockForRead();
    const auto& acts = get()->_actions;
    for ( FaceAction* act : acts)
        act->purge( fm);
    fm->unlock();
    MS::remove(fm); // Removes views
    MM::purge(fm);  // Removes cached metric calculation data
    FMM::close(fm);

    bool movedView = false;
    Vis::FV *nextfv = nullptr;

    // If there are other FaceModels loaded, select one with a preference for models from the default viewer.
    if ( FMM::numOpen() > 0)
    {
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
            if ( nextfv)
            {
                ActionMoveView::move( nextfv, MS::defaultViewer());
                movedView = true;
            }   // end if
        }   // end else
    }   // end if

    if ( nextfv)
        MS::setSelected( nextfv);
    if ( movedView)
        get()->doEvent( Event::VIEWER_CHANGE);
    s_closeLock.unlock();
}   // end close


void FaceActionManager::doEvent( const Event &E)
{
    const FM* fm = MS::selectedModel();
    assert( fm || FMM::numOpen() == 0);
    if ( E == Event::CANCELLED)
        return;

    // The sending action should not be triggered by its own events (note that
    // executed actions always refresh their own state upon completion).
    // NOTE sact may be null since a FaceAction may not be causing this call!
    FaceAction* sact = qobject_cast<FaceAction*>( sender());

#ifndef NDEBUG
    if ( E == Event::NONE && sact)
        std::cerr << "[WARNING]: " << E << " from " << sact->debugName() << std::endl;
#endif

    if ( fm)
    {
        // Purge actions first.
        for ( FaceAction* act : _actions)
            if ( act != sact && act->isPurgeEvent(E))
                act->purge( fm);

        // Geometry change events require rebuilding the actors associated with the affected FaceModels.
        // This also reapplies visualisations and it is necessary that any visualisations that rely upon
        // associations of data be fully purged first which is why the above loop to purge the actions
        // comes before this check.
        if ( has( E, Event::MESH_CHANGE | Event::ASSESSMENT_CHANGE | Event::RESTORE_CHANGE))
            applyFnToModels( E, fm, rebuildViewActors);   // Also resets view normals
        else if ( has( E, Event::AFFINE_CHANGE))
            applyFnToModels( E, fm, pokeViewTransformsWithModel);

        if ( has( E, Event::MESH_CHANGE | Event::AFFINE_CHANGE | Event::ASSESSMENT_CHANGE | Event::LANDMARKS_CHANGE))
            applyFnToModels( E, fm, MS::syncBoundingVisualisation);
    }   // end if

    MS::updateRender();

    // Have actions recheck their own state and whether or not they're enabled, then see
    // if the received event triggers them.
    for ( FaceAction* act : _actions)
    {
        // Sending actions should not be able to trigger themselves, and
        // actions are not reentrant generally speaking so running actions are also ignored.
        if ( act == sact || act->isWorking())
            continue;

        // In refreshing state, actions decide whether to enable themselves and check themselves.
        act->refreshState(E);

        if ( act->isEnabled() && act->isTriggerEvent(E))
        {
            assert( !has( E, Event::NONE));
            act->execute(E);
        }   // end if
    }   // end for

    // Finally notify the client application of updates and re-render.
    emit onUpdate( fm);
    MS::updateRender();
}   // end doEvent
