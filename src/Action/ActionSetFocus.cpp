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

#include <ActionSetFocus.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <algorithm>
using FaceTools::Action::ActionSetFocus;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using MS = FaceTools::Action::ModelSelector;


// public
ActionSetFocus::ActionSetFocus( const QString& dn, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dn, icon, ks)
{
}   // end ctor


bool ActionSetFocus::checkEnable( Event) { return true;}


void ActionSetFocus::setFocus( FMV* vwr, const cv::Vec3f& v)
{
    vwr->setCameraFocus( v);
}   // end setFocus


bool ActionSetFocus::doBeforeAction( Event)
{
    bool go = false;
    if ( MS::isViewSelected())
    {
        QPoint mpos = primedMousePos();
        if ( mpos.x() < 0)
            mpos = MS::mousePos();
        go = MS::selectedView()->isPointOnFace( mpos);
    }   // end if
    return go;
}   // end doBeforeAction


void ActionSetFocus::doAction( Event)
{
    FV* fv = MS::selectedView();
    cv::Vec3f nf;
    QPoint mpos = primedMousePos();
    if ( mpos.x() < 0)
        mpos = MS::mousePos();
    fv->projectToSurface( mpos, nf);
    setFocus( fv->viewer(), nf);
    emit onEvent( Event::CAMERA_CHANGE);
}   // end doAction
