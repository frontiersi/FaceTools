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
#include <ActionDetectFace.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::ActionDetectFace;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;


ActionDetectFace::ActionDetectFace( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _mint(NULL)
{
    init();
    checkEnable();
    setEnabled(false);
}   // end ctor


void ActionDetectFace::setInteractive( ModelInteractor* interactor, bool enable)
{
    _mint = NULL;
    if ( !enable)
        interactor->getView()->disconnect(this);
    else
        _mint = interactor;
    checkEnable();
}   // end setInteractive


bool ActionDetectFace::doAction()
{
    assert( _faceDetector != NULL);
    assert( _mint != NULL);
    if ( _mint->getModel()->isDetected())
    {
        QMessageBox mb( tr("(Re)detect Face"), tr("Overwrite existing detection? This will reset landmarks!"),
                QMessageBox::Warning,
                QMessageBox::Yes,
                QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
                QMessageBox::NoButton);
        if ( mb.exec() == QMessageBox::No)
            return false;
    }   // end if

    return _mint->getModel()->detectFace( _faceDetector);
}   // end doAction


// public
void ActionDetectFace::initFaceDetection( const QString& haarModelsDir, const QString& faceLmksShapeModel)
{
    _faceDetector = FaceTools::FaceDetector::create( haarModelsDir.toStdString(), faceLmksShapeModel.toStdString());
}   // end initFaceDetection


// private slot
void ActionDetectFace::checkEnable()
{
    setEnabled( _mint && _faceDetector != NULL);
}   // end checkEnable

