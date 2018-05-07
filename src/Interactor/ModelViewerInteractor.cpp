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

#include <ModelViewerInteractor.h>
#include <ModelViewer.h>
#include <cassert>
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::ModelViewer;

ModelViewerInteractor::ModelViewerInteractor( ModelViewer* mv)
    : _viewer(NULL)
{
    setViewer(mv);
}   // end ctor


ModelViewerInteractor::~ModelViewerInteractor()
{
    setViewer(NULL);
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
