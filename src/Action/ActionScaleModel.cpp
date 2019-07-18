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

#include <ActionScaleModel.h>
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


bool ActionScaleModel::checkEnable( Event)
{
    return MS::isViewSelected();
}   // ed checkEnabled


bool ActionScaleModel::doBeforeAction( Event)
{
    bool doResize = false;
    _dialog->reset( MS::selectedModel());
    if ( _dialog->exec() > 0)
    {
        doResize = true;
        const double x = _dialog->xScaleFactor();
        const double y = _dialog->yScaleFactor();
        const double z = _dialog->zScaleFactor();
        _smat = cv::Matx44d( x, 0, 0, 0, // Scaling matrix
                             0, y, 0, 0,
                             0, 0, z, 0,
                             0, 0, 0, 1);
        MS::showStatus("Resizing model...");
    }   // end if

    return doResize;
}   // end doBeforeAction


void ActionScaleModel::doAction( Event)
{
    storeUndo(this, Event::AFFINE_CHANGE);
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    fm->addTransformMatrix( _smat);
    fm->unlock();
}   // end doAction


void ActionScaleModel::doAfterAction( Event)
{
    MS::showStatus( "Finished resizing model.", 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    emit onEvent( Event::AFFINE_CHANGE);
}   // end doAfterAction

