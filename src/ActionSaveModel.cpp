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

#include <ActionSaveModel.h>
#include <FaceModelManager.h>   // FaceTools
#include <FaceModel.h>
#include <QFileDialog>
#include <QMessageBox>
#include <boost/filesystem.hpp>
using FaceTools::ActionSaveModel;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;
typedef FaceTools::FaceModelManager FMM;

// public
ActionSaveModel::ActionSaveModel( const std::string& fname)
    : FaceTools::FaceAction(),
      _icon( fname.c_str()),
      _dname( "&Save..."),
      _keyseq( Qt::CTRL + Qt::Key_S),
      _fmodel(NULL)
{
    init();
    checkEnable();
}   // end ctor


void ActionSaveModel::setInteractive( ModelInteractor* interactor, bool enable)
{
    _fmodel = NULL;
    if ( !enable)
        interactor->getModel()->disconnect( this);
    else
    {
        _fmodel = interactor->getModel();
        connect( _fmodel, &FaceModel::onClearedUndos, this, &ActionSaveModel::checkEnable);
    }   // end else
    checkEnable();
}   // end setInteractive


void ActionSaveModel::checkEnable()
{
    setEnabled( _fmodel && _fmodel->hasUndos());
}   // end checkEnable


// protected
bool ActionSaveModel::doAction()
{
    assert( _fmodel);
    return FMM::get().save( _fmodel, _fmodel->getSaveFilepath());
}   // end doAction


