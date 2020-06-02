/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionScaleModel.h>
#include <FaceModel.h>
using FaceTools::Action::ActionScaleModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionScaleModel::ActionScaleModel( const QString &dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks), _dialog(nullptr)
{
    setAsync(true);
}   // end ctor


void ActionScaleModel::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _dialog = new Widget::ResizeDialog(p);
}   // end postInit


bool ActionScaleModel::isAllowed( Event) { return MS::isViewSelected();}


bool ActionScaleModel::doBeforeAction( Event)
{
    _dialog->reset( MS::selectedModel());
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
    }   // end if

    return doResize;
}   // end doBeforeAction


void ActionScaleModel::doAction( Event)
{
    storeUndo(this, Event::AFFINE_CHANGE | Event::CAMERA_CHANGE);
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    fm->addTransformMatrix( _smat);
    fm->unlock();
}   // end doAction


Event ActionScaleModel::doAfterAction( Event)
{
    // Also apply the scale transform to the camera position (focus is fixed)
    const Vec3f cpos = r3d::transform( _smat, MS::selectedViewer()->camera().pos());
    MS::selectedViewer()->setCameraPosition(cpos);

    MS::showStatus( "Finished resizing model.", 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return Event::AFFINE_CHANGE;
}   // end doAfterAction

