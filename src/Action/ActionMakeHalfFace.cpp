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

#include <Action/ActionMakeHalfFace.h>
//#include <MaskRegistration.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <r3d/Slicer.h>
using FaceTools::Action::ActionMakeHalfFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;

using r3d::Vec3f;
using r3d::Mat4f;


namespace {

void reflectLandmarks( FM* fm, const Mat4f& tmat, const Vec3f& p, const Vec3f& n)
{
    const FaceTools::Landmark::LandmarkSet& lmks = fm->currentLandmarks();
    for ( const auto& pair : lmks.lateral( FaceTools::LEFT))
    {
        const int lmid = pair.first;
        const Vec3f lpos = lmks.pos(lmid, FaceTools::LEFT);
        const Vec3f rpos = lmks.pos(lmid, FaceTools::RIGHT);
        // Find which lateral needs to be reflected by testing the dot product
        const float ldot = n.dot(lpos - p);
        const float rdot = n.dot(rpos - p);
        if ( ldot > 0 && rdot < 0)   // Keep the left lateral
            fm->setLandmarkPosition( lmid, FaceTools::RIGHT, r3d::transform( tmat, lpos));
        else if ( rdot > 0 && ldot < 0)   // Keep the right lateral
            fm->setLandmarkPosition( lmid, FaceTools::LEFT, r3d::transform( tmat, rpos));
    }   // end for

    for ( const auto& pair : lmks.lateral( FaceTools::MID))
    {
        const int lmid = pair.first;
        const Vec3f lpos0 = lmks.pos(lmid, FaceTools::MID);
        const Vec3f lpos1 = r3d::transform( tmat, lpos0);
        const Vec3f lpos = fm->findClosestSurfacePoint( (lpos0 + lpos1) * 0.5f);
        fm->setLandmarkPosition( lmid, FaceTools::MID, lpos);
    }   // end for
}   // end reflectLandmarks

}   // end namespace


ActionMakeHalfFace::ActionMakeHalfFace( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico), _n(0,1,0), _p(0,0,0)
{
    setAsync(false);
}   // end ctor


QString ActionMakeHalfFace::toolTip() const
{
    return "Make a symmetric face through the medial plane.";
}   // end toolTip


QString ActionMakeHalfFace::whatsThis() const
{
    QStringList htxt;
    htxt << "Copy one half of the face reflected through the median plane,";
    htxt << "and join it onto the original unreflected half to make a symmetric";
    htxt << "face through the medial plane. If landmarks aren't present, the YZ";
    htxt << "plane through the origin is used as the reflecting plane.";
    return htxt.join(" ");
}   // end whatsThis


void ActionMakeHalfFace::setPlane( const Vec3f& n, const Vec3f& p)
{
    _n = n;
    _p = p;
}   // end setPlane


bool ActionMakeHalfFace::isAllowed( Event) { return MS::isViewSelected();}


bool ActionMakeHalfFace::doBeforeAction( Event)
{
    bool go = true;

    // If the selected model has no landmarks, ask the user if they want to continue.
    const FM* fm = MS::selectedModel();
    if ( fm->currentAssessment()->landmarks().empty())
    {
        static const QString msg = tr("There are no landmarks to place the cutting plane! Use the preset cutting plane instead?");
        QWidget* prnt = static_cast<QWidget*>(parent());
        go = QMessageBox::Yes == QMessageBox::question( prnt, tr("Use preset cutting plane?"), msg,
                                      QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if

    if ( go)
        storeUndo( this, Event::MESH_CHANGE | Event::AFFINE_CHANGE);
    return go;
}   // end doBeforeAction


void ActionMakeHalfFace::doAction( Event)
{
    FM* fm = MS::selectedModel();

    // Set up the reflection in X axis transformation
    static Mat4f tmat = Mat4f::Identity();
    tmat(0,0) = -1;

    const Vec3f p = _p;
    const Vec3f n = _n;

    fm->lockForWrite();

    // Set the model into standard position before doing the reflection.
    const Mat4f T = fm->transformMatrix();   // Copy out for after
    fm->addTransformMatrix( fm->inverseTransformMatrix());

    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
    mesh->fixTransformMatrix();

    r3d::Mesh::Ptr half0 = r3d::Slicer( *mesh)( p, n);       // Copy of one half
    r3d::Mesh::Ptr half1 = half0->deepCopy();  // Copy of half for reflecting
    half1->invertNormals(); // Invert face normals indices on the other half before reflecting.

    // Reflect the copied half
    half1->setTransformMatrix(tmat);
    half1->fixTransformMatrix();

    // Join newly reflected half1 onto half0
    half0->join( *half1, true);
    mesh = half0;

    fm->update( mesh, true, false);

    /*
    // Also do the mask - keep the vertex IDs intact!
    if ( fm->hasMask())
    {
        r3d::Mesh::Ptr mask = fm->mask().deepCopy();
        mask->fixTransformMatrix();

        const auto &maskData = MaskRegistration::maskData();
        const auto &oppVtxs = maskData->oppVtxs;
        const IntSet &l0 = maskData->q0;  // Top left
        const IntSet &l1 = maskData->q3;  // Bottom left

        if ( _n[0] < 0)
        {
            for ( int lvidx : l0)
                mask->adjustRawVertex( lvidx, mask->uvtx( oppVtxs.at( lvidx)));
            for ( int lvidx : l1)
                mask->adjustRawVertex( lvidx, mask->uvtx( oppVtxs.at( lvidx)));
        }   // end if
        else
        {
            for ( int lvidx : l0)
                mask->adjustRawVertex( oppVtxs.at(lvidx), mask->uvtx( lvidx));
            for ( int lvidx : l1)
                mask->adjustRawVertex( oppVtxs.at(lvidx), mask->uvtx( lvidx));
        }   // end else

        fm->setMask( mask);
    }   // end if
    */

    // Also need to update the positions of the lateral landmarks on the rejected side to reflect
    // their partner positions through the same plane. Also reflect the median landmarks and take
    // the average of their original and reflected positions as their new positions.
    reflectLandmarks( fm, tmat, p, n);

    fm->addTransformMatrix( T); // Re-orient.

    fm->unlock();
}   // end doAction


Event ActionMakeHalfFace::doAfterAction( Event)
{
    return Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE | Event::LANDMARKS_CHANGE | Event::PATHS_CHANGE;
}   // end doAfterAction

