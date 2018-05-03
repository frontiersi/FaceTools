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
#include <ObjModelMover.h>  // RFeatures
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
using FaceTools::Action::ActionDetectFace;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

using FaceTools::Detect::FaceDetector;


ActionDetectFace::ActionDetectFace( const QString& haar, const QString& lmks, QWidget *parent)
    : FaceAction(true/*disable before other*/), _icon( ":/icons/DETECT_FACE"), _parent(parent), _detector(NULL), _transformer(NULL)
{
    addChangeTo( MODEL_TRANSFORMED);
    addChangeTo( MODEL_ORIENTATION_CHANGED);

    // Default (parent) implementation of respondToChange adequate for these events.
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);

    if ( FaceDetector::initialise( haar.toStdString(), lmks.toStdString()))
        _detector = new FaceDetector;
    else
        std::cerr << "[WARNING] FaceTools::Action::ActionDetectFace: Failed to initialise FaceTools::Detect::FaceDetector!" << std::endl;
}   // end ctor


ActionDetectFace::~ActionDetectFace()
{
    if ( _detector)
        delete _detector;
}   // end dtor


bool ActionDetectFace::testReady( FaceControl*) { return _detector;}


bool ActionDetectFace::doBeforeAction( FaceControlSet& rset)
{
    bool docheck = false;
    for ( FaceControl* fc : rset)
    {
        if ( FaceTools::hasReqLandmarks( fc->data()->landmarks()))  // Warn if about to overwrite!
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


bool ActionDetectFace::doAction( FaceControlSet& rset)
{
    _failSet.clear();
    FaceControlSet fset = rset; // Copy out
    for ( FaceControl* fc : fset)
    {
        FaceModel* fm = fc->data();
        if ( !fm->kdtree()) // Create the KD-tree if not present
            fm->kdtree() = RFeatures::ObjModelKDTree::create(fm->model());

        RFeatures::Orientation& on = fm->orientation();
        FaceTools::LandmarkSet& lmks = fm->landmarks();
        if ( !_detector->detect( fm->kdtree(), on, lmks))
        {
            _failSet.insert(fc);
            rset.erase(fc);
        }   // end if
    }   // end for
    return !rset.empty();   // Success if at least one detection
}   // end doAction


void ActionDetectFace::doAfterAction( const FaceControlSet& rset, bool v)
{
    if ( !_failSet.empty()) // Warn failure
    {
        QString msg = tr("Face detection failed on the selected face!");
        if ( !rset.empty())
            msg = tr("Face detection failed on one of the selected faces!");
        QMessageBox::warning(_parent, tr("Detection Failed!"), msg);
    }   // end if
    _failSet.clear();

    FaceAction::doAfterAction( rset, v);    // Update render

    if ( !rset.empty() && _transformer) // Finally, transform if set in this action.
        _transformer->chain(rset);
}   // end doAfterAction
