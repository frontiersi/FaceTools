/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionCentreModel.h>
#include <Action/ActionOrientCamera.h>
using FaceTools::Action::ActionCentreModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::ModelSelect;


ActionCentreModel::ActionCentreModel( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    addRefreshEvent( Event::MESH_CHANGE | Event::AFFINE_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionCentreModel::isAllowed( Event) { return MS::isViewSelected();}


bool ActionCentreModel::doBeforeAction( Event e)
{
    const bool dothis = isAllowed( e);
    if ( dothis)
    {
        storeUndo( this, Event::MESH_CHANGE | Event::AFFINE_CHANGE | Event::CAMERA_CHANGE);
        // Get the position to centre at
        const Vis::FV *fv = MS::selectedView();
        _pos = -fv->data()->bounds()[0]->centre();
        QPoint mp = primedMousePos();
        if ( mp.x() >= 0)
        {
            Vec3f mpos;
            if ( fv->projectToSurface( mp, mpos))
                _pos = -mpos;
        }   // end if
    }   // end if
    return dothis;
}   // doBeforeAction


void ActionCentreModel::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    const Mat4f T = fm->transformMatrix();
    Mat4f cT = Mat4f::Identity();
    cT.block<3,1>(0,3) = _pos;
    fm->addTransformMatrix( cT * fm->inverseTransformMatrix());
    //fm->fixTransformMatrix();
    fm->addTransformMatrix(T);
}   // end doAction


Event ActionCentreModel::doAfterAction( Event)
{
    ActionOrientCamera::orient( MS::selectedView(), 1);
    MS::showStatus("Set model centre.", 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return Event::MESH_CHANGE | Event::AFFINE_CHANGE | Event::CAMERA_CHANGE;
}   // end doAfterAction

