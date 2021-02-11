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

#include <Action/ActionResizeModel.h>
#include <FaceModel.h>
using FaceTools::Action::ActionResizeModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionResizeModel::ActionResizeModel( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _dialog(nullptr)
{
    setAsync(true);
}   // end ctor


void ActionResizeModel::postInit()
{
    _dialog = new Widget::ResizeDialog(static_cast<QWidget*>(parent()));
}   // end postInit


bool ActionResizeModel::isAllowed( Event) { return MS::isViewSelected();}


bool ActionResizeModel::doBeforeAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    _dialog->reset( fm.get());
    _smat = Mat4f::Identity();

    if ( _dialog->exec() > 0)
    {
        _smat(0,0) = float(_dialog->xScaleFactor());
        _smat(1,1) = float(_dialog->yScaleFactor());
        _smat(2,2) = float(_dialog->zScaleFactor());
    }   // end if

    bool doResize = false;
    if ( !_smat.isIdentity() && !_smat.isZero())
    {
        MS::showStatus("Resizing model...");
        doResize = true;
        _ev = Event::MESH_CHANGE | Event::AFFINE_CHANGE | Event::CAMERA_CHANGE;
        if ( fm->hasLandmarks())
            _ev |= Event::LANDMARKS_CHANGE;
        if ( fm->hasPaths())
            _ev |= Event::PATHS_CHANGE;
        storeUndo(this, _ev);
    }   // end if

    return doResize;
}   // end doBeforeAction


void ActionResizeModel::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    const Mat4f T = fm->transformMatrix();
    fm->addTransformMatrix( _smat * fm->inverseTransformMatrix());
    fm->fixTransformMatrix();
    fm->addTransformMatrix( T);
}   // end doAction


Event ActionResizeModel::doAfterAction( Event)
{
    // Also apply the scale transform to the camera position (focus is fixed)
    const Vec3f cpos = r3d::transform( _smat, MS::selectedViewer()->camera().pos());
    MS::selectedViewer()->setCameraPosition(cpos);
    MS::showStatus( "Finished resizing model.", 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return _ev;
}   // end doAfterAction

