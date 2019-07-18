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

#include <ActionInvertNormals.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionInvertNormals;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionInvertNormals::ActionInvertNormals( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionInvertNormals::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


bool ActionInvertNormals::doBeforeAction( Event)
{
    MS::showStatus( "Inverting normals on selected model...");
    return true;
}   // end doBeforeAction


void ActionInvertNormals::invertNormals(RFeatures::ObjModel::Ptr model)
{
    const IntSet& fids = model->faces();
    for ( int fid : fids)
        model->reversePolyVertices(fid);
}   // end invertNormals


void ActionInvertNormals::doAction( Event)
{
    storeUndo(this, Event::GEOMETRY_CHANGE);

    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    RFeatures::ObjModel::Ptr model = fm->wmodel();
    invertNormals(model);
    fm->update( nullptr, false);    // Connectivity update not required
    fm->unlock();
}   // end doAction


void ActionInvertNormals::doAfterAction( Event)
{
    MS::showStatus("Finished inverting model normals.", 5000);
    emit onEvent( Event::GEOMETRY_CHANGE);
}   // end doAfterAction

