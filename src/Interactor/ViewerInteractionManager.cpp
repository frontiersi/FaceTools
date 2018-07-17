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

#include <ViewerInteractionManager.h>
#include <ModelViewer.h>
#include <algorithm>
#include <cassert>
using FaceTools::Interactor::ViewerInteractionManager;
using FaceTools::Interactor::ModelViewerEntryExitInteractor;
using FaceTools::ModelViewer;


ViewerInteractionManager::ViewerInteractionManager( ModelViewer *mv) : _activeViewer(mv)
{
    assert(mv);
    addViewer(mv);
}   // end ctor


ViewerInteractionManager::~ViewerInteractionManager()
{
    std::for_each( std::begin(_vmap), std::end(_vmap), [](auto p){ delete p.first;});
}   // end dtor


// public
void ViewerInteractionManager::addViewer( ModelViewer *mv)
{
    auto vint = new ModelViewerEntryExitInteractor(mv);
    _vmap[vint] = mv;
    connect( vint, &ModelViewerEntryExitInteractor::onEnter, this, &ViewerInteractionManager::doOnViewerEntered);
}   // end addViewer


// private slot
void ViewerInteractionManager::doOnViewerEntered()
{
    ModelViewerEntryExitInteractor* vi = qobject_cast<ModelViewerEntryExitInteractor*>( sender());
    ModelViewer* tv = vi->viewer();      // Target viewer (newly active)
    ModelViewer* sv = _activeViewer;
    const size_t mcount = sv->transferInteractors( tv); // Transfer interactors to target viewer
    _activeViewer = tv;
    assert(_activeViewer != nullptr);
    // Transfer will have moved the fixed interactors, so move them back.
    if ( mcount > 0)
        std::for_each( std::begin(_vmap), std::end(_vmap), [](auto p){ p.first->setViewer(p.second);});
    emit onActivatedViewer(_activeViewer);
}   // end doOnViewerEntered
