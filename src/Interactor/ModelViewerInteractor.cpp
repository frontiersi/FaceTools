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

#include <ModelViewerInteractor.h>
#include <ModelViewer.h>
#include <cassert>
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::ModelViewer;

ModelViewerInteractor::ModelViewerInteractor( ModelViewer* mv, QStatusBar* sb)
    : _viewer(nullptr), _sbar(sb), _ilock(0)
{
    setViewer(mv);
}   // end ctor


ModelViewerInteractor::~ModelViewerInteractor()
{
//    setViewer(nullptr);
}   // end dtor


void ModelViewerInteractor::setViewer( ModelViewer* viewer)
{
    if ( viewer == _viewer)
        return;

    if ( _viewer)
    {
        _viewer->detach(this);
        onDetached();
    }   // end if

    _viewer = viewer;

    if ( _viewer)
    {
        _viewer->attach(this);
        onAttached();
    }   // end if
}   // end setViewer


void ModelViewerInteractor::setInteractionLocked( bool dolock)
{
    if ( _viewer)
    {
        _viewer->unlockInteraction(_ilock);
        if ( dolock)
            _ilock = _viewer->lockInteraction();
    }   // end if
}   // end setInteractionLocked


bool ModelViewerInteractor::isInteractionLocked() const
{
    return _viewer && _viewer->isInteractionLocked();
}   // end isInteractionLocked


void ModelViewerInteractor::showStatus( const QString& msg, int tmsecs)
{
    if ( _sbar)
        _sbar->showMessage( msg, tmsecs);
}   // end showStatus


void ModelViewerInteractor::clearStatus()
{
    if ( _sbar)
        _sbar->clearMessage();
}   // end clearStatus

