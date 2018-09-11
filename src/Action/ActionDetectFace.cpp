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
#include <FaceDetector.h>   // FaceTools
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Detect::FaceDetector;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon,
        const QString& haar, const QString& lmks, QWidget *parent, QProgressBar* pb)
    : FaceAction(dn, icon), _parent(parent), _detector(nullptr)
{
    if ( FaceDetector::initialise( haar.toStdString(), lmks.toStdString()))
        _detector = new FaceDetector;
    else
        std::cerr << "[WARNING] FaceTools::Action::ActionDetectFace: FaceDetector failed to initialise!" << std::endl;

    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


ActionDetectFace::~ActionDetectFace()
{
    if ( _detector)
        delete _detector;
}   // end dtor


bool ActionDetectFace::testReady( const FV* fv) { return fv->canTexture();}


bool ActionDetectFace::doBeforeAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FM* fm = fvs.first()->data();

    bool go = true;
    fm->lockForRead(); // Warn if about to overwrite!
    if ( FaceTools::Detect::FaceShapeLandmarks2DDetector::numDetectionLandmarks( fm->landmarks()) > 0)
    {
        static const QString msg = tr("Selected model's face detection will be overwritten! Continue?");
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

    fm->lockForWrite();
    FV* fv = fm->fvs().first(); // Doesn't matter which view's actor we use since it'll be separately rendered

    RFeatures::Orientation on;
    LandmarkSet::Ptr lmks = fm->landmarks();
    if ( _detector->detect( fm->kdtree(), on, lmks))
    {
        //std::cerr << "Detected orientation (norm,up) : " << on << std::endl;
        fm->setOrientation(on);
        using namespace FaceTools::Landmarks;
        cv::Vec3f c = calcFaceCentre( lmks->pos(L_EYE_CENTRE), lmks->pos(R_EYE_CENTRE), lmks->pos(NASAL_TIP));
        fm->setCentre(c);
        fvs.insert(fm);
    }   // end if
    fm->unlock();

    return !fvs.empty();
}   // end doAction


void ActionDetectFace::doAfterAction( EventSet& cset, const FVS& fvs, bool v)
{
    if ( !v) // Warn failure
    {
        QString msg = tr( _detector->err().c_str());
        QMessageBox::warning(_parent, tr("Face Detection Failed!"), msg);
    }   // end if
    cset.insert(ORIENTATION_CHANGE);
    cset.insert(LANDMARKS_CHANGE);
}   // end doAfterAction
