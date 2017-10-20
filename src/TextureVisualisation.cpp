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

#include <TextureVisualisation.h>
#include <FaceModel.h>
#include <VtkActorCreator.h>    // RVTK
#include <cassert>
using FaceTools::TextureVisualisation;
using FaceTools::FaceModel;


TextureVisualisation::TextureVisualisation( const std::string& iconfile, bool isdefault)
    : FaceTools::VisualisationAction( isdefault), _icon(iconfile.c_str())
{
    init(); // FaceTools::FaceAction
}   // end ctor


// public
bool TextureVisualisation::isAvailable( const FaceModel* fmodel) const
{
    return fmodel->getObjectMeta()->getObject()->getNumMaterials() > 0;
}   // end isAvailable


// public
vtkSmartPointer<vtkActor> TextureVisualisation::makeActor( const FaceModel* fmodel)
{
    const RFeatures::ObjModel::Ptr model = fmodel->getObjectMeta()->getObject();
    assert( model->getNumMaterials() == 1);
    RVTK::VtkActorCreator actorCreator;
    std::vector<vtkSmartPointer<vtkActor> > actors;
    actorCreator.generateTexturedActors( model, actors);
    assert( actors.size() == 1);
    return actors[0];
}   // end makeActor


