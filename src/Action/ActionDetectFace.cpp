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

#include <ActionDetectFace.h>
#include <FaceOrientationDetector.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceModelViewer.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::UndoState;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Detect::FaceOrientationDetector;
using FLD = FaceTools::Detect::FaceShapeLandmarks2DDetector;
using FD = FaceTools::Detect::FeaturesDetector;
using FaceTools::Widget::DetectionCheckDialog;
using MS = FaceTools::Action::ModelSelector;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon)
    : FaceAction(dn, icon), _cdialog(nullptr)
{
    setAsync( true);
}   // end ctor


void ActionDetectFace::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _cdialog = new DetectionCheckDialog(p);
}   // end postInit


bool ActionDetectFace::checkEnable( Event)
{
    return FD::isinit() && FLD::isinit() && MS::isViewSelected() && MS::selectedView()->canTexture();
}   // end checkEnabled


bool ActionDetectFace::doBeforeAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForRead();
    _ulmks.clear();
    _err = "";

    if ( fm->landmarks().empty())
        _ulmks = LDMKS_MAN::ids();
    else // Warn if about to overwrite!
    {
        if ( _cdialog->open( fm))    // Modal
            _ulmks = _cdialog->landmarks(); // Copy out
    }   // end if
    fm->unlock();

    const bool doAct = !_ulmks.empty();
    if ( doAct)
        MS::showStatus("Detecting face...");
    return doAct;
}   // end doBeforeAction


void ActionDetectFace::doAction( Event)
{
    //storeUndo( this, {Event::LANDMARKS_CHANGE, Event::ORIENTATION_CHANGE, Event::METRICS_CHANGE}, false);
    storeUndo( this, {Event::LANDMARKS_CHANGE, Event::ORIENTATION_CHANGE, Event::METRICS_CHANGE});
    FM* fm = MS::selectedModel();
    _err = redetectLandmarks( fm, &_ulmks);
}   // end doAction


// public static
std::string ActionDetectFace::redetectLandmarks( FM* fm, const IntSet *ulmks)
{
    Landmark::LandmarkSet::Ptr lmks = Landmark::LandmarkSet::create();
    std::string errstr = "";

    fm->lockForRead();
    FaceOrientationDetector faceDetector( fm, DEFAULT_CAMERA_DISTANCE, 0.3f);
    if ( ulmks) // Specify the set of landmarks to be updated.
        faceDetector.setLandmarksToUpdate( *ulmks);
    const bool detectedOkay = faceDetector.detect( *lmks);
    fm->unlock();

    if ( detectedOkay)
    {
        // The landmarks are transformed by T from standard position.
        const cv::Matx44d T = lmks->orientation().asMatrix( lmks->fullMean());
        const cv::Matx44d Tinv = T.inv();
        // So transform them back into standard position...
        lmks->addTransformMatrix( Tinv);
        lmks->fixTransformMatrix(); // Sets the internal matrix back to I
        // Before setting the transform matrix back. Note that calling FM::addTransformMatrix(Tinv)
        // also adds the inverse of T to lmks meaning that the final transform matrix will be set
        // back to the identity matrix.
        lmks->addTransformMatrix( T);

        fm->lockForWrite();
        fm->setLandmarks(lmks);        // LANDMARKS_CHANGE | FACE_DETECTED
        fm->addTransformMatrix(Tinv);
        fm->fixOrientation();          // ORIENTATION_CHANGE
        fm->clearMetrics();            // METRICS_CHANGE
        fm->unlock();
    }   // end if
    else
        errstr = faceDetector.error();

    return errstr;
}   // end redetectLandmarks


void ActionDetectFace::doAfterAction( Event)
{
    if ( _err.empty())
    {
        MS::clearStatus();
        MS::setInteractionMode( IMode::CAMERA_INTERACTION);
        emit onEvent( {Event::LANDMARKS_CHANGE, Event::FACE_DETECTED, Event::ORIENTATION_CHANGE, Event::METRICS_CHANGE});
    }   // end if
    else
    {
        QMessageBox::warning( static_cast<QWidget*>(parent()), tr("Face Detection Failed!"), tr( _err.c_str()));
        MS::showStatus("Face Detection Failed!", 10000);
    }   // end else
}   // end doAfterAction
