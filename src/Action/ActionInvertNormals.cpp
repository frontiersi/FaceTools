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

#include <Action/ActionInvertNormals.h>
#include <FaceModel.h>
using FaceTools::Action::ActionInvertNormals;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


ActionInvertNormals::ActionInvertNormals( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionInvertNormals::isAllowed( Event) { return MS::isViewSelected();}


bool ActionInvertNormals::doBeforeAction( Event)
{
    MS::showStatus( "Inverting normals on selected model...");
    storeUndo(this, Event::MESH_CHANGE);
    return true;
}   // end doBeforeAction


void ActionInvertNormals::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    mesh->invertNormals();
    fm->update( mesh, false, false);
    fm->unlock();
}   // end doAction


Event ActionInvertNormals::doAfterAction( Event)
{
    MS::showStatus("Finished inverting model normals.", 5000);
    return Event::MESH_CHANGE;
}   // end doAfterAction

