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


VisualisationAction::VisualisationAction()
    : FaceAction()
{
    setEnabled(false);
}   // end ctor


// protected slot
bool VisualisationAction::doAction()
{
    foreach ( ModelInteractor* mint, _interactors)
        mint->getView()->visualise(this);
    return true;
}   // end doAction


// private slot
void VisualisationAction::doOnMeshUpdated()
{
    bool enable = !_interactors.empty();
    // Only enable if ALL of the interactors allow for this visualisation
    foreach ( ModelInteractor* mint, _interactors)
    {
        if ( mint->getView()->canVisualise(this))
        {
            enable = false;
            break;
        }   // end if
    }   // end foreach
    setEnabled( enable);
}   // end doOnMeshUpdated


// public
void VisualisationAction::removeInteractor( ModelInteractor *mint)
{
    _interactors.erase(mint);
    mint->getModel()->disconnect(this);
}   // end removeInteractor


// public
void VisualisationAction::setInteractive( ModelInteractor* mint, bool enable)
{
    if ( !enable)
        removeInteractor(mint);
    else
    {
        _interactors.insert(mint);
        connect( mint->getModel(), &FaceModel::onMeshUpdated, this, &VisualisationAction::doOnMeshUpdated);
    }   // end else if
    doOnMeshUpdated();
}   // end checkAllow
