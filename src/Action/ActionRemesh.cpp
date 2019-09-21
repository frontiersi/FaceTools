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

#include <Action/ActionRemesh.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelVertexAdder.h>   // RFeatures
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionRemesh;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionRemesh::ActionRemesh( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _maxtarea(2.0)
{
    setAsync(true);
}   // end ctor


bool ActionRemesh::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


bool ActionRemesh::doBeforeAction( Event)
{
    MS::showStatus( "Remeshing model...");
    return true;
}   // end doBeforeAction

void ActionRemesh::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    RFeatures::ObjModel::Ptr model = fm->model()->deepCopy(true);
    RFeatures::ObjModelVertexAdder vadder( model);
    vadder.subdivideAndMerge( maxTriangleArea());
    //vadder.addVerticesToMaxTriangleArea( maxTriangleArea());
    fm->update(model);
    fm->moveLandmarksToSurface();
    fm->unlock();
}   // end doAction


void ActionRemesh::doAfterAction()
{
    MS::showStatus( "Finished remeshing model.", 5000);
    emit onEvent( Event::GEOMETRY_CHANGE);
}   // end doAfterAction
