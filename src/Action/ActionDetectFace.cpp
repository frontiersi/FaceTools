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
#include <r3d/VectorPCFinder.h>
#include <r3d/ProcrustesSuperimposition.h>
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
    : FaceAction(dn, icon), _cdialog(nullptr)
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
    bool goDetect = true;

    if ( fm->currentLandmarks().empty())
        _ulmks = Landmark::LandmarksManager::ids();
    else if ( _cdialog->open( fm))    // Modal dialog - warn if about to overwrite!
        _ulmks = _cdialog->landmarks(); // Copy out
    else
        goDetect = false;

    if ( goDetect)
        MS::showStatus("Detecting face - please wait ...");
    return goDetect;
}   // end doBeforeAction


void ActionDetectFace::doAction( Event)
{
    storeUndo( this, Event::LANDMARKS_CHANGE
                   | Event::MESH_CHANGE
                   | Event::MASK_CHANGE
                   | Event::AFFINE_CHANGE);
    _err = detectLandmarks( MS::selectedModel(), _ulmks);
}   // end doAction


namespace {

void setMask( r3d::Mesh::Ptr msk, FM *fm)
{
    fm->setMask( msk);
    using FaceTools::MaskRegistration;
    // Don't set the full path - just the filename.
    const std::string maskfile = boost::filesystem::path( MaskRegistration::maskPath().toStdString()).filename().string();
    fm->setMaskFilename( QString::fromStdString( maskfile));
    fm->setMaskHash( MaskRegistration::maskHash());
}   // end setMask


r3d::Mat4f calcMaskAlignment( const r3d::Mesh &mask)
{
    const FaceTools::MaskRegistration::MaskPtr mdata = FaceTools::MaskRegistration::maskData();

    const IntSet &vidxs = mdata->mask->mesh().vtxIds();
    r3d::MatX3f tgtVtxRows( vidxs.size(), 3);
    r3d::MatX3f mvtxs( vidxs.size(), 3);
    int i = 0;
    for ( int vidx : vidxs)
    {
        tgtVtxRows.row(i) = mdata->mask->mesh().vtx(vidx);
        mvtxs.row(i) = mask.vtx(vidx);
        i++;
    }   // end for

    r3d::VecXf weights = r3d::VecXf::Ones( vidxs.size());
    r3d::ProcrustesSuperimposition psupp( tgtVtxRows, weights);
    r3d::Mat4f T = psupp( mvtxs);

    return T;
}   // end calcMaskAlignment

}   // end namespace


// public static
std::string ActionDetectFace::detectLandmarks( FM* fm, const IntSet &ulmks)
{
    // If the caller provides a set of landmark IDs to update, it should not be empty!
    if ( ulmks.empty())
        return "Empty landmark update set specified!";

    // Clone the existing set for update
    fm->lockForWrite();
    fm->fixTransformMatrix(); // Ensure any existing transform is fixed in place first

    std::cout << "Calculating initial alignment of mask to target face..." << std::endl;
    const Mat4f T1 = ActionAlignModel::calcAlignmentTransform( fm, false); // Apply for initial rigid alignment (no scaling).
    r3d::Mesh::Ptr mask = MaskRegistration::registerMask( fm, T1);

    std::cout << "Updating target face correspondences and transferring landmarks..." << std::endl;
    setMask( mask, fm);
    ActionRestoreLandmarks::restoreLandmarks( fm, ulmks);

    std::cout << "Aligning face from mask..." << std::endl;

    const Mat4f align = calcMaskAlignment( *mask);
    const Mat4f ialign = align.inverse();
    fm->addTransformMatrix( ialign);
    fm->fixTransformMatrix();
    fm->unlock();

    return "";
}   // end detectLandmarks


Event ActionDetectFace::doAfterAction( Event)
{
    Event e = Event::NONE;
    if ( _err.empty())
    {
        MS::clearStatus();
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        e |= Event::LANDMARKS_CHANGE | Event::MESH_CHANGE | Event::MASK_CHANGE | Event::AFFINE_CHANGE;
        MS::showStatus( "Detected face and placed landmarks.", 5000);
    }   // end if
    else
    {
        QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Face Detection Failed!"), tr( _err.c_str()));
        MS::showStatus("Face Detection Failed!", 10000);
    }   // end else
    return e;
}   // end doAfterAction
