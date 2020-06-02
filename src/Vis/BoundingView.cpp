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

#include <Vis/BoundingView.h>
using FaceTools::Vis::BoundingView;
using FaceTools::Vis::SimpleView;


BoundingView::BoundingView( float lw)
{
    vtkActor *actor = addActor(_cubeSource);
    vtkProperty* prop = actor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetLineWidth( lw);
}   // end ctor


void BoundingView::update( const r3d::Vec6f &cb)
{
    _cubeSource->SetBounds( cb[0], cb[1], cb[2], cb[3], cb[4], cb[5]);
    _cubeSource->Update();
}   // end update
