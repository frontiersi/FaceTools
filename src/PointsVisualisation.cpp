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

#include <PointsVisualisation.h>
#include <FaceModel.h>
#include <VtkActorCreator.h>
#include <vtkProperty.h>
#include <vtkMapper.h>
using FaceTools::PointsVisualisation;

PointsVisualisation::PointsVisualisation( const std::string& iconfile, bool isdefault)
    : FaceTools::VisualisationAction( isdefault), _icon( iconfile.c_str())
{
    init();
}   // end ctor


// public
vtkSmartPointer<vtkActor> PointsVisualisation::makeActor( const FaceTools::FaceModel* fmodel)
{
    RVTK::VtkActorCreator actorCreator;
    vtkSmartPointer<vtkActor> actor = actorCreator.generateSurfaceActor(fmodel->getObjectMeta()->getObject());
    actor->GetProperty()->SetRepresentationToPoints();
    actor->GetMapper()->SetScalarVisibility(false);
    return actor;
}   // end makeActor


