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

#include <Interactor/ViewerNotifier.h>
#include <Action/ModelSelector.h>
#include <ModelViewer.h>
using FaceTools::Interactor::ViewerNotifier;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


ViewerNotifier::ViewerNotifier() : _vwr(nullptr)
{
    for ( FMV* mv : MS::viewers())
    {
        _vwrs[mv->getRenderWindow()] = mv;
        static_cast<ModelViewer*>(mv)->attach(this);
    }   // end for
    _vwr = MS::defaultViewer();
}   // end ctor


ViewerNotifier::~ViewerNotifier()
{
    for ( FMV* mv : MS::viewers())
        static_cast<ModelViewer*>(mv)->detach(this);
    _vwrs.clear();
}   // end dtor


void ViewerNotifier::mouseEnter( const QTools::VtkActorViewer *v)
{
    assert(_vwrs.count(v->getRenderWindow()) > 0);
    FMV* vwr = _vwrs.at(v->getRenderWindow());
    _vwr = vwr;
    enterViewer(vwr);
}   // end mouseEnter


void ViewerNotifier::mouseLeave( const QTools::VtkActorViewer *v)
{
    assert(_vwrs.count(v->getRenderWindow()) > 0);
    FMV* vwr = _vwrs.at(v->getRenderWindow());
    if ( _vwr == vwr)
        leaveViewer(vwr);
}   // end mouseLeave


FV* ViewerNotifier::viewFromActor( const vtkProp3D* prop) const
{
    FV* fv = nullptr;
    FMV* fmv = static_cast<FMV*>(mouseViewer());
    if ( fmv)
    {
        fv = fmv->attached().find(prop);
        if ( fv && fv->actor() != prop)
            fv = nullptr;
    }   // end if
    return fv;
}   // end viewFromActor
