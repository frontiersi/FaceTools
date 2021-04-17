/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <ModelSelect.h>
#include <Interactor/SelectNotifier.h>
#include <FileIO/FaceModelManager.h>
#include <FaceModelViewer.h>
#include <Metric/MetricManager.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <functional>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FMM = FaceTools::FileIO::FaceModelManager;
using MS = FaceTools::ModelSelect;
using MM = FaceTools::Metric::MetricManager;
using FaceTools::FM;
//#undef NDEBUG


namespace {

void rebuildViews( FM* fm) { fm->rebuildViews();}


void syncViews( FM* fm)
{
    vtkSmartPointer<vtkMatrix4x4> m = r3dvis::toVTK( fm->transformMatrix());
    for ( FV* fv : fm->fvs())
        fv->pokeTransform( m);
}   // end syncViews


void applyFnToModels( const Event &e, FM* fm, const std::function<void( FM*)> &fn)
{
    if ( !has( e, Event::ALL_VIEWS))
        fn( fm);
    else
    {
        const FaceTools::FVS& aset = MS::selectedView()->viewer()->attached();
        for ( FM* vm : aset.models())
            fn( vm);
    }   // end else
}   // end applyFnToModels

}   // end namespace


FaceActionManager::Ptr FaceActionManager::s_singleton;


FaceActionManager::FaceActionManager() : _lvl(0)
{
    connect( this, &FaceActionManager::_selfRaise, this, &FaceActionManager::_doRaise);
}   // end ctor


// static
FaceActionManager* FaceActionManager::get()
{
    if ( !s_singleton)
        s_singleton = Ptr(new FaceActionManager, [](FaceActionManager* fam){ delete fam;});
    return &*s_singleton;
}   // end get


// static
QAction* FaceActionManager::registerAction( FaceAction* act, QWidget *parentWidget)
{
    std::vector<FaceAction*>& acts = get()->_actions;
    acts.push_back(act);
    act->addRefreshEvent( Event::MODEL_SELECT | Event::CLOSED_MODEL);
    act->_init( parentWidget);    // No more events added after this
    return act->qaction();
}   // end registerAction


// static
void FaceActionManager::finalise()
{
    FaceActionManager *fam = get();
    std::vector<FaceAction*>& acts = fam->_actions;

    for ( size_t i = 0; i < acts.size(); ++i)
    {
        FaceAction *act = acts.at(i);
        act->refresh();
        connect( act, &FaceAction::onEvent, fam, &FaceActionManager::_doRaise);
        connect( act, &FaceAction::onShowHelp, [fam](const QString& tok){ emit fam->onShowHelp(tok);});
    }   // end for
    const Interactor::SelectNotifier *sn = MS::selectNotifier();
    connect( sn, &Interactor::SelectNotifier::onSelected, []( Vis::FV*, bool s){ if ( s) raise( Event::MODEL_SELECT);});
}   // end finalise


// static
FaceTools::Vis::FV* FaceActionManager::close( const FM *fm)
{
    assert(fm);
    FaceActionManager *fam = get();
    fm->lockForRead();
    const auto& acts = fam->_actions;
    for ( FaceAction* act : acts)
        act->purge( fm);
    fm->unlock();
    MS::remove(fm); // Removes views which ordinarily would cause Event::MODEL_SELECT but signal is blocked
    MM::purge(fm);  // Removes cached metric calculation data
    FMM::close(*fm);

    Vis::FV *nextfv = nullptr;

    // If there are other FaceModels loaded, select one with a preference for models from the default viewer.
    if ( FMM::numOpen() > 0)
    {
        if ( !MS::defaultViewer()->empty())
            nextfv = MS::defaultViewer()->attached().first();
        else
        {
            // Get the next FaceView to select from one of the other viewers.
            for ( const FMV* fmv : MS::viewers())
            {
                if ( !fmv->empty())
                {
                    nextfv = fmv->attached().first();
                    break;
                }   // end if
            }   // end for
        }   // end else
    }   // end if

    return nextfv;
}   // end close


// static
void FaceActionManager::raise( Event E)
{
    // emit to queue in the signal handling thread.
    // Don't call _doRaise directly since the client might
    // have called raise from a non-GUI thread.
    emit get()->_selfRaise( E);
}   // end raise


void FaceActionManager::_doRaise( Event E)
{
    FM* fm = MS::selectedModel();
    assert( fm || FMM::numOpen() == 0);
    if ( E == Event::CANCEL)
        return;

    // The sending action should not be triggered by its own events (note that
    // executed actions always refresh their own state upon completion).
    // NOTE sact may be null since a FaceAction may not be causing this call!
    FaceAction* sact = qobject_cast<FaceAction*>( sender());

    if ( fm && E != Event::NONE)
    {
        // Purge actions first.
        for ( FaceAction *act : _actions)
            if ( act != sact && act->purges( E) && !act->isWorking())
                act->purge( fm);
        if ( has( E, Event::MESH_CHANGE))
            applyFnToModels( E, fm, rebuildViews);   // Also resets view normals
        else if ( has( E, Event::AFFINE_CHANGE))
            applyFnToModels( E, fm, syncViews);
    }   // end if

    // Note that handlers can own visualisations and refreshing handlers can adjust
    // the visualisations they own. Since ActionVisualise looks at the visualisation
    // availability and visibility in its refresh function, this must be called first.
    MS::refreshHandlers();

    std::vector<FaceAction*> tacts; // Actions to be triggered as an immediate response to event(s) E
    std::vector<FaceAction*> racts; // Actions to refresh afterwards due to E (if not triggered by E)
    _acted[sact] = E; // Sending actions cannot respond to themselves (set with their own finishing event)

    if ( E != Event::NONE)
    {
        for ( FaceAction *act : _actions)
        {
            // Actions are not reentrant so running actions are ignored
            // (they refresh at the end of their action in any case).
            if ( act->isWorking())
                continue;

            // Actions that have already been acted upon (or that are scheduled
            // for refresh already) are ignored to prevent recursion - only
            // the event info is updated for when the action refreshes.
            if ( _acted.count(act) > 0)
            {
                _acted[act] |= E;
                continue;
            }   // end if

            if ( act->triggers( E)) // Triggers take precedence
            {
                tacts.push_back(act);
                _acted[act] = E;
            }   // end if
            else if ( act->refreshes( E))
            {
                racts.push_back(act);
                _acted[act] = E;
            }   // end else if
        }   // end for
    }   // end if

    const std::string chevrons( ++_lvl, '>');

    // Actions triggered for immediate response by the received action (if we have any)
    if ( !tacts.empty())
    {
#ifndef NDEBUG
        std::cerr << chevrons << " ===== Actions triggered by " << E << " ===== " << std::endl;
#endif
        for ( FaceAction *act : tacts)
        {
            act->_setDebugPrefix( chevrons);
            act->execute(E);
            act->_setDebugPrefix( "!");
        }   // end for
    }   // end if

    if ( !racts.empty())
    {
#ifndef NDEBUG
        if ( !racts.empty())
            std::cerr << chevrons << " ===== Refreshing " << racts.size() << " actions =====" << std::endl;
#endif
        for ( FaceAction *act : racts)
        {
            assert( act);
#ifndef NDEBUG
            std::cerr << chevrons << " " << act->debugName() << " due to ";
            assert( _acted.count(act) > 0);
            const Event re = _acted.at(act) & act->refreshEvents();
            std::cerr << re << std::endl;
#endif
            // Ensure the action is refreshed with the complete set of events received
            // and not just the event that caused it to be identified as a refresh event.
            act->refresh( _acted.at(act));
        }   // end for
    }   // end if
    
    _lvl--;
    if ( _lvl == 0)
    {
        _acted.clear();
        MS::updateRender();
        emit onUpdateSelected();
    }   // end if
}   // end _doRaise
