/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
using MS = FaceTools::Action::ModelSelector;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon)
    : FaceAction(dn, icon), _cdialog(nullptr), _ev(Event::NONE)
{
    setAsync( true);
}   // end ctor


void ActionDetectFace::postInit()
{
    _cdialog = new LandmarksCheckDialog( static_cast<QWidget*>(parent()));
}   // end postInit


bool ActionDetectFace::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    return fm && Detect::FaceAlignmentFinder::isInit()
           && MaskRegistration::maskLoaded()
           && fm->mesh().hasSequentialIds();
}   // end isAllowed


bool ActionDetectFace::doBeforeAction( Event)
{
    FM* fm = MS::selectedModel();
    _ulmks.clear();
    _err = "";
    _ev = Event::NONE;
    bool goDetect = true;

    if ( fm->currentLandmarks().empty())
        _ulmks = Landmark::LandmarksManager::ids();
    else if ( _cdialog->open( fm))    // Modal dialog - warn if about to overwrite!
        _ulmks = _cdialog->landmarks(); // Copy out
    else
        goDetect = false;

    if ( goDetect)
    {
        MS::showStatus("Detecting face - please wait ...");
        _ev = Event::MESH_CHANGE | Event::MASK_CHANGE | Event::AFFINE_CHANGE;
        if ( !_ulmks.empty())
            _ev |= Event::LANDMARKS_CHANGE;
        storeUndo( this, _ev);
    }   // end if

    return goDetect;
}   // end doBeforeAction


void ActionDetectFace::doAction( Event)
{
    FM *fm = MS::selectedModel();
    fm->lockForWrite();
    _err = detect( fm, _ulmks);
    fm->unlock();
}   // end doAction


// public static
std::string ActionDetectFace::detect( FM* fm, const IntSet &ulmks)
{
    std::cout << "Doing initial alignment of model..." << std::endl;
    ActionAlignModel::align( fm);
    fm->fixTransformMatrix();
    std::cout << "Registering mask against target face..." << std::endl;
    r3d::Mesh::Ptr mask = MaskRegistration::registerMask( fm);

    std::cout << "Setting mask..." << std::endl;
    fm->setMask( mask);
    fm->setMaskHash( MaskRegistration::maskHash());

    if ( !ulmks.empty())
    {
        std::cout << "Transferring landmarks..." << std::endl;
        ActionRestoreLandmarks::restoreLandmarks( fm, ulmks);
    }   // end if

    std::cout << "Doing Procrustes alignment of the coregistered mask with the original..." << std::endl;
    const Mat4f align = MaskRegistration::calcMaskAlignment( *mask);
    const Mat4f ialign = align.inverse();

    fm->addTransformMatrix( ialign);
    fm->fixTransformMatrix();
    return "";
}   // end detect


Event ActionDetectFace::doAfterAction( Event)
{
    if ( _err.empty())
    {
        MS::clearStatus();
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        MS::showStatus( "Detected face and placed landmarks.", 5000);
    }   // end if
    else
    {
        QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Face Detection Failed!"), tr( _err.c_str()));
        MS::showStatus("Face Detection Failed!", 10000);
    }   // end else
    return _ev;
}   // end doAfterAction
