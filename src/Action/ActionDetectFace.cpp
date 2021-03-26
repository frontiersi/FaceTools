/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionDetectFace.h>
#include <Action/ActionAlignModel.h>
#include <Action/ActionRestoreLandmarks.h>
#include <Action/ActionOrientCamera.h>
#include <Detect/FaceAlignmentFinder.h>
#include <LndMrk/LandmarksManager.h>
#include <boost/filesystem/path.hpp>
#include <MaskRegistration.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::LandmarksCheckDialog;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon)
    : FaceAction(dn, icon), _cdialog(nullptr), _ev(Event::NONE), _success(false)
{
    setAsync( true);
}   // end ctor


void ActionDetectFace::postInit()
{
    _cdialog = new LandmarksCheckDialog( static_cast<QWidget*>(parent()));
}   // end postInit


bool ActionDetectFace::isAllowed( Event)
{
    const FM::RPtr fm = MS::selectedModelScopedRead();
    return fm && Detect::FaceAlignmentFinder::isInit()
           && MaskRegistration::maskLoaded()
           && fm->mesh().hasSequentialIds();
}   // end isAllowed


bool ActionDetectFace::doBeforeAction( Event)
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    _ulmks.clear();
    _ev = Event::NONE;
    bool goDetect = true;

    if ( fm->currentLandmarks().empty())
        _ulmks = Landmark::LandmarksManager::ids();
    else if ( _cdialog->open( *fm))    // Modal dialog - warn if about to overwrite!
        _ulmks = _cdialog->landmarks(); // Copy out
    else
        goDetect = false;

    if ( goDetect)
    {
        MS::showStatus("Detecting face - please wait ...");
        _ev = Event::MESH_CHANGE | Event::MASK_CHANGE | Event::AFFINE_CHANGE | Event::VIEW_CHANGE;
        if ( !_ulmks.empty())
            _ev |= Event::LANDMARKS_CHANGE;
        storeUndo( this, _ev);
    }   // end if

    _success = false;
    return goDetect;
}   // end doBeforeAction


// public static
bool ActionDetectFace::detect( FM &fm, const IntSet &ulmks, bool alignFirst)
{
    if ( alignFirst)
        ActionAlignModel::align( fm);

    fm.fixTransformMatrix();

    r3d::Mesh::Ptr mask = MaskRegistration::registerMask( fm.kdtree());
    if ( !mask)
        return false;

    fm.setMask( mask);
    fm.setMaskHash( MaskRegistration::maskHash());

    if ( !ulmks.empty())
        ActionRestoreLandmarks::restoreLandmarks( fm, ulmks, false);

    const Mat4f align = MaskRegistration::calcMaskAlignment( *mask);
    const Mat4f ialign = align.inverse();
    fm.addTransformMatrix( ialign);
    fm.fixTransformMatrix();
    return true;
}   // end detect


void ActionDetectFace::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    _success = detect( *fm, _ulmks, true);
}   // end doAction


Event ActionDetectFace::doAfterAction( Event)
{
    MS::clearStatus();
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    Event ev = Event::NONE;
    if ( _success)
    {
        ActionOrientCamera::orient( MS::selectedView(), 1);
        QString plusLmks;
        if ( has( _ev, Event::LANDMARKS_CHANGE))
            plusLmks = " and placed landmarks";
        MS::showStatus( QString("Detected face%1.").arg(plusLmks), 5000);
        ev = _ev | Event::CAMERA_CHANGE;
    }   // end if
    else
    {
        MS::showStatus( "Detection failed!", 10000);
        static const QString msg = tr("Failed to detect the face!<br>Try removing non-face model parts first.");
        QMessageBox::warning( static_cast<QWidget*>(parent()),
                tr("Detection Failed!"), QString("<p align='center'>%1</p>").arg(msg));
    }   // end else
    _success = false;
    return ev;
}   // end doAfterAction
