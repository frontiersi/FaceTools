/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <MaskRegistration.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <r3d/Slicer.h>
using FaceTools::Action::ActionMakeHalfFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;
using r3d::Vec3f;
using r3d::Mat4f;


namespace {

void reflectLandmarks( FM &fm, const Mat4f& tmat, const Vec3f& p, const Vec3f& n)
{
    const FaceTools::Landmark::LandmarkSet& lmks = fm.currentLandmarks();
    for ( const auto& pair : lmks.lateral( FaceTools::LEFT))
    {
        const int lmid = pair.first;
        const Vec3f lpos = lmks.pos(lmid, FaceTools::LEFT);
        const Vec3f rpos = lmks.pos(lmid, FaceTools::RIGHT);
        // Find which lateral needs to be reflected by testing the dot product
        const float ldot = n.dot(lpos - p);
        const float rdot = n.dot(rpos - p);
        if ( ldot > 0 && rdot < 0)   // Keep the left lateral
            fm.setLandmarkPosition( lmid, FaceTools::RIGHT, r3d::transform( tmat, lpos));
        else if ( rdot > 0 && ldot < 0)   // Keep the right lateral
            fm.setLandmarkPosition( lmid, FaceTools::LEFT, r3d::transform( tmat, rpos));
    }   // end for

    for ( const auto& pair : lmks.lateral( FaceTools::MID))
    {
        const int lmid = pair.first;
        const Vec3f lpos0 = lmks.pos(lmid, FaceTools::MID);
        const Vec3f lpos1 = r3d::transform( tmat, lpos0);
        const Vec3f lpos = fm.findClosestSurfacePoint( (lpos0 + lpos1) * 0.5f);
        fm.setLandmarkPosition( lmid, FaceTools::MID, lpos);
    }   // end for
}   // end reflectLandmarks

}   // end namespace


ActionMakeHalfFace::ActionMakeHalfFace( const QString &dn, const QIcon& ico)
    : FaceAction( dn, ico), _n(0,1,0), _p(0,0,0), _ev(Event::NONE)
{
    setAsync(true);
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
    _ev = Event::MESH_CHANGE | Event::AFFINE_CHANGE;

    // If the selected model has no landmarks, ask the user if they want to continue.
    FM::RPtr fm = MS::selectedModelScopedRead();
    if ( fm->hasLandmarks())
        _ev |= Event::LANDMARKS_CHANGE;
    else
    {
        static const QString msg = tr("Facial landmarks are not present and so cannot be used to find the centreline. Slice through the YZ plane at X = 0 instead?");
        go = QMB::Yes == QMB::question( static_cast<QWidget*>(parent()), tr("Use Default Slicing Plane?"),
                QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No);
    }   // end else

    //if ( fm->hasMask())
    //    _ev |= Event::MASK_CHANGE;
    if ( fm->hasPaths())
        _ev |= Event::PATHS_CHANGE;

    if ( go)
        storeUndo( this, _ev);
    return go;
}   // end doBeforeAction


void ActionMakeHalfFace::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();

    // Set up the reflection in X axis transformation
    static Mat4f tmat = Mat4f::Identity();
    tmat(0,0) = -1;

    const Vec3f p = _p; // Plane position
    const Vec3f n = _n; // Plane normal

    // Set the model into standard position before doing the reflection.
    const Mat4f T = fm->transformMatrix();   // Copy out for transforming back afterwards
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

    // Also do the mask - keep the vertex IDs intact!
    if ( fm->hasMask())
    {
        const r3d::Mesh &omask = fm->mask();
        r3d::Mesh::Ptr mask = omask.deepCopy();
        mask->fixTransformMatrix();

        const auto &maskData = MaskRegistration::maskData();
        const auto &oppVtxs = maskData->oppVtxs;

        const IntSet *l0;
        const IntSet *l1;
        if ( _n[0] < 0)
        {
            l0 = &maskData->q0;  // Top left
            l1 = &maskData->q3;  // Bottom left
        }   // end if
        else
        {
            l0 = &maskData->q1;  // Top right
            l1 = &maskData->q2;  // Bottom right
        }   // end else

        for ( int lvidx : *l0)
        {
            const Vec3f &v = omask.uvtx( oppVtxs.at( lvidx));
            mask->adjustRawVertex( lvidx, -v[0], v[1], v[2]);
        }   // end for
        for ( int lvidx : *l1)
        {
            const Vec3f &v = omask.uvtx( oppVtxs.at( lvidx));
            mask->adjustRawVertex( lvidx, -v[0], v[1], v[2]);
        }   // end for

        fm->setMask( mask);
    }   // end if

    // Also need to update the positions of the lateral landmarks on the rejected side to reflect
    // their partner positions through the same plane. Also reflect the median landmarks and take
    // the average of their original and reflected positions as their new positions.
    reflectLandmarks( *fm, tmat, p, n);

    fm->addTransformMatrix( T); // Re-orient.
}   // end doAction


Event ActionMakeHalfFace::doAfterAction( Event) { return _ev;}

