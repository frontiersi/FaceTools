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

#include <VisualisationAction.h>
#include <ModelInteractor.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::VisualisationAction;
using FaceTools::ModelInteractor;
using FaceTools::FaceModel;
using FaceTools::FaceView;


VisualisationAction::VisualisationAction() : FaceAction(), _fview(NULL), _fmodel(NULL)
{
    setEnabled(false);
}   // end ctor


// protected slot
bool VisualisationAction::doAction()
{
    assert(_fview);
    _fview->visualise(this);
    return true;
}   // end doAction


// public
void VisualisationAction::connectInteractor( ModelInteractor* interactor)
{
    assert( interactor);
    connect( interactor, SIGNAL( enableActionable(bool)), this, SLOT(checkAllow(bool)));
}   // end connectInteractor


// public
void VisualisationAction::disconnectInteractors()
{
    if ( _fmodel)
        _fmodel->disconnect(this);
    _fview = NULL;
    _fmodel = NULL;
}   // end disconnectInteractor


// private slot
void VisualisationAction::doOnMeshUpdated()
{
    setEnabled( _fview && _fview->canVisualise(this));
}   // end doOnMeshUpdated


// private slot
void VisualisationAction::checkAllow( bool enable)
{
    ModelInteractor* mint = qobject_cast<ModelInteractor*>( sender());
    assert(mint);
    if ( !enable && (mint->getView() == _fview))
        disconnectInteractors();
    else if ( enable && !_fview)
    {
        _fview = mint->getView();
        _fmodel = mint->getModel();
        connect( _fmodel, SIGNAL( onMeshUpdated()), this, SLOT(doOnMeshUpdated()));
    }   // end else if
    doOnMeshUpdated();
}   // end checkAllow

