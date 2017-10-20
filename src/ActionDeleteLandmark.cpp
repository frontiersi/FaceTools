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

#include <ActionDeleteLandmark.h>
#include <ModelInteractor.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::ActionDeleteLandmark;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;


ActionDeleteLandmark::ActionDeleteLandmark( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _interactor(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionDeleteLandmark::setInteractive( ModelInteractor* interactor, bool enable)
{
    _interactor = NULL;
    if ( !enable)
        interactor->disconnect( this);
    else
    {
        _interactor = interactor;
        connect( _interactor, &ModelInteractor::onEnteringLandmark, this, &ActionDeleteLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingLandmark, this, &ActionDeleteLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onEnteringModel, this, &ActionDeleteLandmark::checkEnable);
        connect( _interactor, &ModelInteractor::onExitingModel, this, &ActionDeleteLandmark::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractor


bool ActionDeleteLandmark::doAction()
{
    // Check if user really wants to delete the landmark
    QMessageBox mb( tr("Delete Landmark"), tr(("Really delete landmark '" + _delLandmark + "'?").c_str()),
                    QMessageBox::Warning,
                    QMessageBox::Yes,
                    QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
                    QMessageBox::NoButton);
    if ( mb.exec() == QMessageBox::No)
        return false;

    _interactor->getModel()->updateLandmark( _delLandmark, NULL);
    _delLandmark = "";
    return true;
}   // end doAction


// private slot
void ActionDeleteLandmark::checkEnable()
{
    std::string lm;
    bool canDelete = false;
    if ( _interactor)
    {
        const QPoint& p = _interactor->getMouseCoords();
        lm = _interactor->getView()->isLandmarkPointedAt(p);
        canDelete = !lm.empty() &&
                    !_interactor->isDrawingPath() &&
                    !_interactor->isMovingLandmark() &&
                    _interactor->getView()->getModel()->getObjectMeta()->getLandmarkMeta(lm)->deletable;
    }   // end if

    _delLandmark = lm;
    setText( tr("Delete landmark"));
    setEnabled( canDelete);
    if ( canDelete)
        setText( tr(("Delete landmark '" + lm + "'").c_str()));
}   // checkEnable


