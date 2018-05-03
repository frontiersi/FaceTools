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
    : _viewer(mv)
{
    const bool rval = mv->attachInteractor(this);
    assert(rval);
}   // end ctor


ModelViewerInteractor::~ModelViewerInteractor()
{
    const bool rval = _viewer->detachInteractor(this);
    assert(rval);
}   // end dtor


// protected
void ModelViewerInteractor::setInteractionLocked( bool v) { _viewer->setInteractionLocked(v);}
bool ModelViewerInteractor::isInteractionLocked() const { return _viewer->isInteractionLocked();}
