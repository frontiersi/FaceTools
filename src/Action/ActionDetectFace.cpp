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

#include <ActionDetectFace.h>
#include <FaceOrientationDetector.h>
#include <FaceShapeLandmarks2DDetector.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;
using FaceTools::Detect::FaceOrientationDetector;
using FLD = FaceTools::Detect::FaceShapeLandmarks2DDetector;
using FD = FaceTools::Detect::FeaturesDetector;
using FaceTools::Widget::DetectionCheckDialog;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon, QWidget *parent, QProgressBar* pb)
    : FaceAction(dn, icon), _parent(parent), _cdialog( new DetectionCheckDialog(parent))
{
    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionDetectFace::testReady( const FV* fv) { return fv->canTexture();}

bool ActionDetectFace::testEnabled( const QPoint*) const { return FD::isinit() && FLD::isinit() && ready1();}


bool ActionDetectFace::doBeforeAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FM* fm = fvs.first()->data();
    fm->lockForRead();
    _ulmks.clear();
    _err = "";

    if ( fm->landmarks()->empty())
        _ulmks = LDMKS_MAN::ids();
    else // Warn if about to overwrite!
    {
        if ( _cdialog->open( fm))    // Modal
            _ulmks = _cdialog->landmarks(); // Copy out
    }   // end if
    fm->unlock();
    return !_ulmks.empty();
}   // end doBeforeAction


bool ActionDetectFace::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FM* fm = fvs.first()->data();
    fvs.clear();
    _err = redetectLandmarks( fm, &_ulmks);
    if ( _err.empty())
        fvs.insert(fm);
    return _err.empty();
}   // end doAction


// public static
std::string ActionDetectFace::redetectLandmarks( FM* fm, const IntSet *ulmks)
{
    fm->lockForWrite();

    const RFeatures::ObjModelKDTree* kdt = fm->kdtree();
    FaceOrientationDetector faceDetector( kdt, 650.0f, 0.3f);
    Landmark::LandmarkSet& lmks = *fm->landmarks();
    fm->clearMeta();

    // Specifiy the set of landmarks to be updated.
    if ( ulmks)
        faceDetector.setLandmarksToUpdate( *ulmks);

    std::string errstr = "";
    if ( faceDetector.detect( lmks))
    {
        const RFeatures::Orientation on = lmks.orientation();
        const cv::Vec3f centre = lmks.fullMean();
        cv::Matx44d m = RFeatures::toStandardPosition( on.nvec(), on.uvec(), centre);
        fm->transform(m);
    }   // end if
    else
        errstr = faceDetector.error();

    fm->unlock();
    return errstr;
}   // end redetectLandmarks


void ActionDetectFace::doAfterAction( EventSet& cset, const FVS&, bool v)
{
    if ( v)
        cset.insert(FACE_DETECTED);
    else
        QMessageBox::warning(_parent, tr("Face Detection Failed!"), tr( _err.c_str()));

    cset.insert(ORIENTATION_CHANGE);
    cset.insert(LANDMARKS_ADD);
    cset.insert(AFFINE_CHANGE);
}   // end doAfterAction
