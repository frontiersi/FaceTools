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

#include <ActionCropBoundary.h>
#include <FaceView.h>
#include <ObjMetaData.h>    // FaceTools
#include <QInputDialog>
#include <QMessageBox>
#include <cassert>
using FaceTools::ActionCropBoundary;
using FaceTools::ModelInteractor;


ActionCropBoundary::ActionCropBoundary( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _interactor(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionCropBoundary::setInteractive( ModelInteractor* interactor, bool enable)
{
    _interactor = NULL;
    if ( !enable)
    {
        interactor->getModel()->disconnect(this);
        interactor->getView()->disconnect(this);
    }   // end if
    else
    {
        _interactor = interactor;
        connect( _interactor->getModel(), &FaceTools::FaceModel::onFaceDetected, this, &ActionCropBoundary::checkEnable);
        connect( _interactor->getView(), &FaceTools::FaceView::onShowBoundary, this, &ActionCropBoundary::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


bool ActionCropBoundary::doAction()
{
    assert( _interactor);
    const bool cropped = _interactor->getModel()->cropFace();
    if ( !cropped)
    {
        QMessageBox mberr( tr("Error cropping!"), tr( _interactor->getModel()->err().c_str()),
                QMessageBox::Warning, QMessageBox::Ok, QMessageBox::NoButton, QMessageBox::NoButton);
        mberr.exec();
    }   // end if
    return cropped;
}   // end doAction


// private slot
void ActionCropBoundary::checkEnable()
{
    if ( !_interactor)
        setEnabled(false);
    else
    {
        const FaceTools::FaceView* fview = _interactor->getView();
        bool enable = fview && fview->isBoundaryShown();
        setEnabled( enable);
    }   // end else
}   // end checkEnable


