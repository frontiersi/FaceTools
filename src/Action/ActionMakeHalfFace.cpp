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

#include <ActionMakeHalfFace.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <ObjModelTools.h>
using FaceTools::Action::ActionMakeHalfFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

using RFeatures::ObjModel;
using RFeatures::ObjModelSlicer;
using RFeatures::Transformer;


ActionMakeHalfFace::ActionMakeHalfFace( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico), _n(0,1,0), _p(0,0,0), _useLmks(false)
{
    setAsync(true);
}   // end ctor


QString ActionMakeHalfFace::toolTip() const
{
    return "Copy one side of the face to the other to make a symmetric face.";
}   // end toolTip


QString ActionMakeHalfFace::whatsThis() const
{
    return "Make a copy of one side of the face, then reflect it through the median plane before joining it back onto the original unreflected half.";
}   // end whatsThis


void ActionMakeHalfFace::setPlane( const cv::Vec3f& n, const cv::Vec3f& p)
{
    _n = n;
    _p = p;
}   // end setPlane


void ActionMakeHalfFace::setUseLandmarksIfPossible(bool v) { _useLmks = v;}


bool ActionMakeHalfFace::checkEnable( Event)
{
    return MS::isViewSelected();
}   // end checkEnabled


bool ActionMakeHalfFace::doBeforeAction( Event)
{
    bool go = true;
    // If this action is set to use landmarks but the selected model has none,
    // ask the user if they want to continue.
    const FM* fm = MS::selectedModel();
    if ( _useLmks && fm->currentAssessment()->landmarks().empty())
    {
        static const QString msg = tr("There are no landmarks to place the cutting plane! Use the preset cutting plane instead?");
        QWidget* prnt = static_cast<QWidget*>(parent());
        go = QMessageBox::Yes == QMessageBox::question( prnt, tr("Use preset cutting plane?"), msg,
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if
    return go;
}   // end doBeforeAction


void ActionMakeHalfFace::doAction( Event)
{
    storeUndo( this, { Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});
    FM* fm = MS::selectedModel();

    // Set up the reflection transformation
    static const cv::Matx44d rmat ( -1,  0,  0,  0,
                                     0,  1,  0,  0,
                                     0,  0,  1,  0,
                                     0,  0,  0,  1);
    cv::Vec3f p = _p;

    fm->lockForWrite();
    // Use cutting plane position as through mean of landmarks if requested and available
    const Landmark::LandmarkSet& lmks = fm->currentAssessment()->landmarks();
    if ( _useLmks && !lmks.empty())
        p = lmks.fullMean();

    ObjModel::Ptr nmod = ObjModelSlicer( fm->model())( p, _n);  // Copy of one half
    ObjModel::Ptr nmod1 = nmod->deepCopy(true/*share mats*/);   // Flipped copy will become other half

    // Invert face normals indices on the other half before reflecting through YZ plane.
    const IntSet& fids = nmod1->faces();
    for ( int fid : fids)
        nmod1->reversePolyVertices(fid);

    nmod1->setTransformMatrix(rmat);
    nmod1->fixTransformMatrix();

    RFeatures::join( *nmod, *nmod1, true); // Join nmod1 to nmod as new vertices
    fm->update(nmod);

    // Also need to update the positions of the lateral landmarks on the rejected side
    // to reflect their partner positions through the same plane.
    const auto& lmksLat = lmks.lateral( FACE_LATERAL_LEFT);
    for ( const auto& pair : lmksLat)
    {
        const int lmid = pair.first;
        const cv::Vec3f lpos = lmks.pos(lmid, FACE_LATERAL_LEFT);
        const cv::Vec3f rpos = lmks.pos(lmid, FACE_LATERAL_RIGHT);
        // Find which lateral needs to be reflected by testing the dot product
        const float ldot = _n.dot(lpos - p);
        const float rdot = _n.dot(rpos - p);
        if ( ldot > 0 && rdot < 0)   // Keep the left lateral
            fm->setLandmarkPosition( lmid, FACE_LATERAL_RIGHT, RFeatures::transform( rmat, lpos));
        else if ( rdot > 0 && ldot < 0)   // Keep the right lateral
            fm->setLandmarkPosition( lmid, FACE_LATERAL_LEFT, RFeatures::transform( rmat, rpos));
    }   // end for

    fm->unlock();
}   // end doAction


void ActionMakeHalfFace::doAfterAction( Event)
{
    emit onEvent( { Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE, Event::LANDMARKS_CHANGE});
}   // end doAfterAction

