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

#include <ActionEditLandmarks.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::Event;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::LandmarksInteractor;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : ActionVisualise( dn, ico, _vis = new LandmarksVisualisation, ks)
{
    LandmarksInteractor *lint = new LandmarksInteractor( *_vis);
    connect( lint, &LandmarksInteractor::onUpdated, [this](){ emit onEvent( Event::LANDMARKS_CHANGE);});
    _interactor = std::shared_ptr<LandmarksInteractor>( lint);
    addTriggerEvent( Event::LOADED_MODEL);
    addTriggerEvent( Event::FACE_DETECTED);
    addTriggerEvent( Event::LANDMARKS_CHANGE);
}   // end ctor


ActionEditLandmarks::~ActionEditLandmarks()
{
    delete _vis;
}   // end dtor


bool ActionEditLandmarks::checkState( Event e)
{
    const bool chk = ActionVisualise::checkState( e);
    _interactor->setEnabled(chk);
    _vis->setHighlighted( MS::selectedModel());
    return chk;
}   // end checkState


bool ActionEditLandmarks::doBeforeAction( Event)
{
    _vis->updateLandmarks( MS::selectedModel());
    return true;
}   // end doBeforeAction


void ActionEditLandmarks::doAfterAction( Event)
{
    if ( isChecked())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( "Move landmarks by left-clicking and dragging.");
    }   // end if
    else
        MS::clearStatus();
}   // end doAfterAction
