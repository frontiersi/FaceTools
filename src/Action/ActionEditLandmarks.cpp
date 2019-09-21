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

#include <Action/ActionEditLandmarks.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditLandmarks;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::LandmarksHandler;
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::Action::ModelSelector;

Q_DECLARE_METATYPE( FaceTools::Landmark::LandmarkSet::Ptr)


ActionEditLandmarks::ActionEditLandmarks( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : ActionVisualise( dn, ico, _vis = new LandmarksVisualisation, ks)
{
    LandmarksHandler *lint = new LandmarksHandler( *_vis);
    connect( lint, &LandmarksHandler::onStartedDrag, [this](){ storeUndo(this);});
    connect( lint, &LandmarksHandler::onFinishedDrag, [this](){ emit onEvent( Event::LANDMARKS_CHANGE);});
    _handler = std::shared_ptr<LandmarksHandler>( lint);
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
    _handler->setEnabled(chk);
    const FM* fm = MS::selectedModel();
    _vis->setHighlighted( fm);  // un-highlights all before highlighting only model's landmarks if not null
    _vis->syncLandmarks( fm);
    return chk;
}   // end checkState


bool ActionEditLandmarks::doBeforeAction( Event)
{
    return true;//!EventGroup(e).is(Event::ASSESSMENT_CHANGE);
}   // end doBeforeAction


void ActionEditLandmarks::doAfterAction( Event)
{
    if ( isChecked())
    {
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        if ( MS::selectedModel()->currentAssessment()->hasLandmarks())
            MS::showStatus( "Move landmarks by left-clicking and dragging.");
    }   // end if
    else
        MS::clearStatus();
}   // end doAfterAction


UndoState::Ptr ActionEditLandmarks::makeUndoState() const
{
    UndoState::Ptr us = UndoState::create( this, Event::LANDMARKS_CHANGE);
    us->setName( "Move Landmark");
    Landmark::LandmarkSet::Ptr lmks = us->model()->currentAssessment()->landmarks().deepCopy();
    us->setUserData( "Lmks", QVariant::fromValue(lmks));
    return us;
}   // end makeUndoState


void ActionEditLandmarks::restoreState( const UndoState* us)
{
    Landmark::LandmarkSet::Ptr lmks = us->userData("Lmks").value<Landmark::LandmarkSet::Ptr>();
    us->model()->setLandmarks(lmks);
    _vis->syncLandmarks( us->model());
}   // end restoreState
