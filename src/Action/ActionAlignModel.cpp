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

#include <Action/ActionAlignModel.h>
#include <MaskRegistration.h>
#include <Action/ActionOrientCamera.h>
#include <Detect/FaceAlignmentFinder.h>
#include <r3d/FrontFinder.h>
#include <FaceModelCurvatureStore.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
using FaceTools::Action::ActionAlignModel;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::Mat3f;
using FaceTools::MatX3f;
using FaceTools::Mat4f;
using FMCS = FaceTools::FaceModelCurvatureStore;
using MS = FaceTools::ModelSelect;


ActionAlignModel::ActionAlignModel( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction(dn, ico, ks)
{
    // Refresh on SURFACE_DATA_CHANGE for FaceModelCurvatureStore completion
    addRefreshEvent( Event::AFFINE_CHANGE | Event::MASK_CHANGE | Event::MESH_CHANGE | Event::SURFACE_DATA_CHANGE);
    setAsync(true);
}   // end ctor


QString ActionAlignModel::whatsThis() const
{
    QStringList htxt;
    htxt << "Align the selected model using its landmarks if present. Otherwise, detect the orientation";
    htxt << "via eye detection on the face texture, falling back to an estimate of the orientation";
    htxt << "using purely geometric means.";
    return htxt.join(" ");
}   // end whatsThis


bool ActionAlignModel::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    const bool isInit = fm && Detect::FaceAlignmentFinder::isInit() && FMCS::rvals(*fm) != nullptr;
    // Enable if the model isn't already aligned, OR if the model is aligned but has no mask (and
    // therefore no capacity to have a separate alignment matrix not equal to the model's orientation).
    return isInit && (!fm->hasMask() || !fm->isAligned());
}   // end isAllowed


bool ActionAlignModel::doBeforeAction( Event)
{
    _egrp = Event::CAMERA_CHANGE | Event::AFFINE_CHANGE | Event::MESH_CHANGE;
    storeUndo( this, _egrp);
    MS::showStatus( "Aligning model...");
    return true;
}   // end doBeforeAction


namespace {

Vec3f makePositive( const Vec3f &v, int i) { return v[i] < 0 ? -v : v;}


Mat4f makeMatrixFromEigenVectors( const MatX3f &nrms, const Vec3f &pos, const Vec3f &nrm)
{
    const r3d::VectorPCFinder pcfinder( nrms);
    const Mat3f E = pcfinder.eigenVectors();

    // The largest (first) eigen vector should be the X vector since faces are taller
    // than wide so there are more horizontal vertex normals than vertical ones.
    Mat4f T = Mat4f::Identity();
    T.block<3,1>(0,0) = makePositive( E.col(0), 0);
    // The norm vector will be the one most in the direction of nrm
    int yid = 1;
    int zid = 2;
    if ( fabsf(E.col(1).dot(nrm)) > fabsf(E.col(2).dot(nrm)))
        std::swap(yid,zid);
    T.block<3,1>(0,1) = makePositive( E.col(yid), 1);
    T.block<3,1>(0,2) = makePositive( E.col(zid), 2);
    T.block<3,1>(0,3) = pos;
    return T;
}   // end makeMatrixFromEigenVectors


// Note that pos must be the current transformed position on the model.
MatX3f getNormFeatures( const FM &fm, Vec3f &pos, float sqRad)
{
    std::vector<std::pair<size_t, float> > vidxs;
    fm.kdtree().findr( pos, sqRad, vidxs);
    const size_t N = vidxs.size();

    const Mat4f T = fm.mesh().transformMatrix();   // Need to transform vertex normals
    FMCS::RPtr curv = FMCS::rvals( fm);
    assert( curv);
    const MatX3f &vnrms = curv->vals().vertexNormals();    // Untransformed
    MatX3f frows( N, 3);
    pos = Vec3f::Zero();
    for ( size_t i = 0; i < N; ++i)
    {
        const int vidx = int(vidxs[i].first);
        pos += fm.mesh().vtx( vidx);   // Transformed position returned
        frows.block<1,3>(i,0) = r3d::transform( T, vnrms.row(vidx));
    }   // end for
    pos /= N;   // New mean position

    return frows;
}   // end getNormFeatures


Mat4f calcGeometricAlignment( const FM &fm, float rad)
{
    r3d::FrontFinder fff( fm.mesh());
    Vec3f pos = fff();
    const Vec3f nrm = fff.meanNrm();
    // Get the vertex normals and update the mean position.
    const MatX3f nrms = getNormFeatures( fm, pos, rad*rad);
    // Get the eigenvector decomposition of the vertex norms
    return makeMatrixFromEigenVectors( nrms, pos, nrm);
}   // end calcGeometricAlignment
}   // end namespace

/*
void ActionAlignModel::alignUsingMaskLandmarksProcrustes( FM *fm)
{
    // Get extrinstic alignment established via Procrustes between
    // this model's landmarks and the loaded mask's landmarks.
    if ( fm->hasLandmarks() && MaskRegistration::maskLoaded())
    {
        MaskRegistration::MaskPtr mdata = MaskRegistration::maskData();
        const MatX3f maskVtxs = mdata->mask->currentLandmarks().toMesh()->vertices2Matrix();
        const MatX3f modelVtxs = fm->currentLandmarks().toMesh()->vertices2Matrix();
        const VecXf W = VecXf::Ones( maskVtxs.rows());
        const Mat4f T = r3d::ProcrustesSuperimposition( maskVtxs, W, false)( modelVtxs);
        fm->addTransformMatrix( T);
        std::cout << "Procrustes result (landmarks):" << std::endl;
        std::cout << T << std::endl;
    }   // end if
    fm->fixTransformMatrix();
}   // end alignUsingMaskLandmarksProcrustes
*/


void ActionAlignModel::align( FM &fm)
{
    if ( fm.hasMask())
    {
        //const Mat4f T = fm.inverseTransformMatrix(); // If model has mask, then simply restore alignment defined by it.
        const r3d::Mat4f T = MaskRegistration::calcMaskAlignment( fm.mask()).inverse();
        fm.addTransformMatrix( T);
    }   // end if
    else
    {
        assert( FMCS::rvals( fm) != nullptr);

        Mat4f T = Mat4f::Zero();

        Detect::FaceAlignmentFinder faf;
        // Texture aligning may fail or model may not have a texture returning zero matrix
        if ( fm.hasTexture())
            T = faf.find( fm.kdtree(), fm.bounds()[0]->centre(), 600.0f);
        else
        {
            T = calcGeometricAlignment( fm, 80);
            assert( !T.isZero());
        }   // end else

        // If previous texture alignment failed (due to being unable to detect the eyes), it's possible that the
        // new initial alignment derived from just looking at the central features will place the model in a
        // better orientation so that texture alignment works this time.
        size_t tries = 0;
        while ( T.isZero() && tries < 5)
        {
            tries++;
            T = calcGeometricAlignment( fm, 80);
            assert( !T.isZero());
            fm.addTransformMatrix( T.inverse());
            T = faf.find( fm.kdtree(), fm.bounds()[0]->centre(), 600.0f);    // May return zero matrix
        }   // end if

        if ( !T.isZero())
            fm.addTransformMatrix( T.inverse());
    }   // end else
}   // end align


void ActionAlignModel::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    align( *fm);
    fm->fixTransformMatrix();
}   // end doAction


Event ActionAlignModel::doAfterAction( Event)
{
    Vis::FV *fv = MS::selectedView();
    ActionOrientCamera::orient( fv, 1);

    QString msg = "Alignment defined by mask";
    if ( !fv->rdata()->hasMask())
        msg = "Alignment recalculated";

    MS::setInteractionMode( IMode::CAMERA_INTERACTION);
    MS::showStatus( msg, 5000);
    return _egrp;
}   // end doAfterAction
