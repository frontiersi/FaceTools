/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

using FaceTools::Detect::FaceDetector;


ActionDetectFace::ActionDetectFace( const QString& dn, const QIcon& icon,
        const QString& haar, const QString& lmks, QWidget *parent, QProgressBar* pb)
    : FaceAction(dn, icon), _parent(parent), _detector(NULL)
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


bool ActionDetectFace::doBeforeAction( FaceControlSet& rset, const QPoint&)
{
    bool docheck = false;
    for ( FaceControl* fc : rset)
    {
        FaceModel* fm = fc->data();
        // Warn if about to overwrite!
        if ( FaceTools::Detect::FaceShapeLandmarks2DDetector::numDetectionLandmarks( fm->landmarks()) > 0)
        {
            docheck = true;
            break;
        }   // end if
    }   // end for

    bool go = true;
    if ( docheck)
    {
        QString msg = tr("Selected model's face detection will be overwritten! Continue?");
        if ( rset.size() > 1)
            msg = tr("Selected models' face detections will be overwritten! Continue?");

        // Check go?
        go = QMessageBox::Yes == QMessageBox::question( _parent, tr("Overwrite face detection(s)?"), msg,
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if
    return go;
}   // end doBeforeAction


bool ActionDetectFace::doAction( FaceControlSet& rset, const QPoint&)
{
    _failSet.clear();
    FaceControlSet sset;

    for ( FaceControl* fc : rset)
    {
        FaceModel* fm = fc->data();
        fm->lockForWrite();

        RFeatures::Orientation on;
        if ( _detector->detect( fm->kdtree(), on, fm->landmarks()))
        {
            std::cerr << "Detected orientation (norm,up) : " << on << std::endl;
            fm->setOrientation(on);
            sset.insert(fc);
        }   // end if
        else
            _failSet.insert(fm);

        fm->unlock();
    }   // end for

    rset = sset;
    return !rset.empty();   // Success if at least one detection
}   // end doAction


void ActionDetectFace::doAfterAction( ChangeEventSet& cset, const FaceControlSet& rset, bool v)
{
    if ( !_failSet.empty()) // Warn failure
    {
        QString msg = tr("Face detection failed on the selected face! Move the face into a different position and try again.");
        if ( !rset.empty())
            msg = tr("Face detection failed on one of the selected faces! Move the faces into a different position and try again.");
        QMessageBox::warning(_parent, tr("Face Detection Failed!"), msg);
    }   // end if
    _failSet.clear();
    cset.insert(LANDMARKS_CHANGE);
    cset.insert(ORIENTATION_CHANGE);
}   // end doAfterAction
