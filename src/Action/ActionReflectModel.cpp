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

#include <Action/ActionReflectModel.h>
#include <MaskRegistration.h>
#include <FaceModel.h>
using FaceTools::Action::ActionReflectModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Action::UndoState;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionReflectModel::ActionReflectModel( const QString &dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction( dn, ico, ks)
{
    setAsync(true);
}   // end ctor


QString ActionReflectModel::toolTip() const
{
    return "Reflect through the YZ plane, or if landmarks are present, through the midsagittal (median) plane.";
}   // end toolTip


bool ActionReflectModel::isAllowed( Event) { return MS::isViewSelected();}


void ActionReflectModel::saveState( UndoState &us) const
{
    us.setUserData( "Mesh", QVariant::fromValue( us.model()->mesh().deepCopy()));
    us.setUserData( "Ass", QVariant::fromValue( us.model()->currentAssessment()->deepCopy()));
}   // end saveState


void ActionReflectModel::restoreState( const UndoState &us)
{
    us.model()->update( us.userData("Mesh").value<r3d::Mesh::Ptr>(), false, false);
    us.model()->setAssessment( us.userData("Ass").value<FaceAssessment::Ptr>());
}   // end restoreState


bool ActionReflectModel::doBeforeAction( Event)
{
    // Even though its an affine change on the model, we need to treat it as a geometry change
    // for the purposes of visualisation since VTK will need to rebuild its normals.
    _ev = Event::MESH_CHANGE | Event::MASK_CHANGE;
    storeUndo( this, _ev, false);
    return true;
}   // end doBeforeAction


namespace {
void swapMaskLaterals( r3d::Mesh &mask)
{
    IntSet rset;
    const auto &oppVtxs = FaceTools::MaskRegistration::maskData()->oppVtxs;
    for ( const std::pair<int,int> &vpair : oppVtxs)
    {
        const int lvidx = vpair.first;
        const int rvidx = vpair.second;
        assert( lvidx >= 0 && rvidx >= 0);
        if ( rset.count(lvidx) == 0)
        {
            assert( rset.count( rvidx) == 0);
            mask.swapVertexPositions( lvidx, rvidx);
        }   // end if
        rset.insert(lvidx);
        rset.insert(rvidx);
    }   // end for
}   // end swapMaskLaterals
}   // end namespace


void ActionReflectModel::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();

    const Mat4f T = fm->transformMatrix();  // Remember for translating back afterwards
    Mat4f rmat = Mat4f::Identity();
    rmat(0,0) = -1;
    rmat = rmat * fm->inverseTransformMatrix(); // Undo current transform then reflect through YZ plane

    // When adding the matrix, it is also applied to the mask, but we don't want this since this will
    // reflect the anthropometric correspondence through the laterals so we first swap the lateral vertex indices.
    // For efficiency we just unset the mask and deal with it later.

    r3d::Mesh::Ptr mask;
    size_t mhash = 0;
    if ( fm->hasMask())
    {
        mask = fm->mask().deepCopy();
        mhash = fm->maskHash();
        fm->setMask( nullptr);
    }   // end if

    fm->addTransformMatrix( rmat);
    if ( fm->hasLandmarks())
        fm->swapLandmarkLaterals();

    r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();    // Clone out
    mesh->invertNormals();
    mesh->fixTransformMatrix();

    fm->update( mesh, false, false);
    if ( mask)
    {
        swapMaskLaterals( *mask);           // So that post reflection the vertex IDs are on the same laterals
        mask->addTransformMatrix( rmat);    // Do the reflection
        mask->fixTransformMatrix();
        fm->setMask( mask);
        fm->setMaskHash( mhash);
    }   // end if

    fm->addTransformMatrix(T);  // Revert back to original position
}   // end doAction


Event ActionReflectModel::doAfterAction( Event)
{
    QString smsg = "Reflected model through YZ plane.";
    if ( MS::selectedModelScopedRead()->hasMask())
        smsg = "Reflected face through mid-sagittal plane.";
    MS::showStatus( smsg, 5000);
    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    return _ev;
}   // end doAfterAction
