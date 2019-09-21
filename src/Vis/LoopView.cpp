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

#include <Vis/LoopView.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
using FaceTools::Vis::LoopView;
using FaceTools::ModelViewer;


LoopView::LoopView( const RFeatures::ObjModel& model, double lw) : _model(model), _lw(lw) {}


void LoopView::add( const std::list<int>& vidxs, double r, double g, double b, double a)
{
    // The returned actor has the model's current transform matrix poked to it.
    vtkActor* actor = RVTK::VtkActorCreator::generateLineActor( _model, vidxs, true);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( _lw);
    actor->SetPickable(false);
    property->SetColor( r, g, b);
    property->SetOpacity( a);
    initActor( actor);
}   // end add

