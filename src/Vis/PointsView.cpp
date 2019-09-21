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

#include <Vis/PointsView.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::PointsView;
using FaceTools::ModelViewer;


PointsView::PointsView( const RFeatures::ObjModel& model, const IntSet& vidxs, double ps, double r, double g, double b, double a)
{
    // The returned actor has the model's current transform matrix applied to it.
    vtkActor* actor = RVTK::VtkActorCreator::generatePointsActor( model, vidxs);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToPoints();
    property->SetPointSize( ps);
    actor->SetPickable(false);
    initActor( actor);
    setColour( r, g, b, a);
}   // end ctor

