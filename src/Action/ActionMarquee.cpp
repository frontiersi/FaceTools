/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ActionMarquee.h>
#include <FaceModelViewer.h>
#include <algorithm>
#include <functional>
using FaceTools::Action::ActionMarquee;
using FaceTools::Action::CameraWorker;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::Interactor::ModelMoveInteractor;
using FaceTools::FVS;
using CW = FaceTools::Action::CameraWorker;


// public
ActionMarquee::ActionMarquee( const QString& dn, const QIcon& ico, ModelMoveInteractor* mmi)
    : FaceAction( dn, ico)
{
    setCheckable(true, false);
    std::function<void()> pred = [this](){ if ( isChecked()) process(false);};
    connect( mmi, &ModelMoveInteractor::onCameraMove, pred);
    connect( mmi, &ModelMoveInteractor::onActorMove, pred);
    setRespondToEvent( CAMERA_CHANGE, false);
}   // end ctor


ActionMarquee::~ActionMarquee()
{
    std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){delete cw;});
}   // end dtor


void ActionMarquee::addViewer( FaceModelViewer* v)
{
    _workers.insert( new CameraWorker( v));
}   // end addViewer


bool ActionMarquee::doAction( FVS&, const QPoint&)
{
    if ( isChecked())
        std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->start();});
    else
        std::for_each( std::begin(_workers), std::end(_workers), [](CW* cw){cw->stop();});
    return true;
}   // end doAction
