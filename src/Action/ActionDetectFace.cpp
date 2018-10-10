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


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon, QWidget *parent, QProgressBar* pb)
    : FaceAction(dn, icon), _parent(parent)
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

    bool go = true;
    fm->lockForRead(); // Warn if about to overwrite!
    if ( !fm->landmarks()->empty())
    {
        static const QString msg = tr("Really overwrite existing landmark detection and orientation?");
        go = QMessageBox::Yes == QMessageBox::question( _parent, tr("Overwrite face detection(s)?"), msg,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if
    fm->unlock();
    return go;
}   // end doBeforeAction


bool ActionDetectFace::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FM* fm = fvs.first()->data();
    fvs.clear();

    // Get a new orientation for the face
    fm->lockForRead();
    const RFeatures::ObjModelKDTree::Ptr kdt = fm->kdtree();
    FaceOrientationDetector faceDetector( kdt);
    const bool oriented = faceDetector.orient();
    fm->unlock();

    if ( !oriented)
    {
        _err = faceDetector.error();
        return false;
    }   // end if

    std::cerr << "[INFO] FaceTools::Action::ActionDetectFace: Detected orientation (norm, up) " << faceDetector.orientation() << std::endl;
    std::cerr << "[INFO] FaceTools::Action::ActionDetectFace: Landmarks detection range set to " << faceDetector.detectRange() << std::endl;

    if ( !faceDetector.detect( *fm->landmarks()))
    {
        _err = faceDetector.error();
        return false;
    }   // end if

    fvs.insert(fm);
    fm->lockForWrite();
    fm->setCentre( FaceTools::calcFaceCentre( *fm->landmarks()));
    fm->setOrientation( faceDetector.orientation());
    fm->unlock();
    return true;
}   // end doAction


void ActionDetectFace::doAfterAction( EventSet& cset, const FVS&, bool v)
{
    if ( !v) // Warn failure
        QMessageBox::warning(_parent, tr("Face Detection Failed!"), tr( _err.c_str()));
    cset.insert(ORIENTATION_CHANGE);
    cset.insert(LANDMARKS_ADD);
}   // end doAfterAction
