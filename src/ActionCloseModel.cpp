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

#include <ActionCloseModel.h>
#include <FaceModelManager.h>
#include <FaceModel.h>
#include <QMessageBox>
using FaceTools::ActionCloseModel;
using FaceTools::FaceModel;
using FaceTools::ModelInteractor;
typedef FaceTools::FaceModelManager FMM;


// public
ActionCloseModel::ActionCloseModel()
    : FaceTools::FaceAction(),
      _keyseq( Qt::CTRL + Qt::Key_W),
      _fmodel(NULL)
{
    init();
    checkEnable();
}   // end ctor


// public
void ActionCloseModel::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fmodel = NULL;
    if ( !enable)
        interactor->getModel()->disconnect( this);
    else
        _fmodel = interactor->getModel();
    checkEnable();
}   // end setInteractive


// protected
bool ActionCloseModel::doAction()
{
    assert(_fmodel);
    if ( _fmodel->hasUndos())
    {
        // Unsaved changes
        QMessageBox mb( tr("Close Model"), tr("Model has unsaved changes! Really close?"),
                        QMessageBox::Warning,
                        QMessageBox::Yes,
                        QMessageBox::No | QMessageBox::Escape | QMessageBox::Default,
                        QMessageBox::NoButton);
        if ( mb.exec() == QMessageBox::No)
            return false;
    }   // end if
    return FMM::get().close( _fmodel);
}   // end doAction


void ActionCloseModel::checkEnable()
{
    setEnabled( _fmodel);
}   // end checkEnable


