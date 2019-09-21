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

#include <Action/ActionNonRigidRegistration.h>
#include <FileIO/BackgroundModelLoader.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelKNNCorresponder.h>
#include <QMessageBox>
#include <QFileInfo>
#include <QThread>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionNonRigidRegistration;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;
using RFeatures::ObjModel;

QString ActionNonRigidRegistration::s_maskPath;
ObjModel::Ptr ActionNonRigidRegistration::s_mask;
QReadWriteLock ActionNonRigidRegistration::s_lock;
int ActionNonRigidRegistration::s_knn(1);


// static
bool ActionNonRigidRegistration::setMaskPath( const QString &mpath)
{
    if ( !FileIO::FMM::canRead( mpath.toStdString()))
        return false;

    s_lock.lockForRead();
    const QString abspath = QFileInfo( mpath).absoluteFilePath();
    const bool samePath = abspath == s_maskPath;
    s_lock.unlock();

    if ( !samePath)
    {
        std::cerr << "Loading template mask for non-rigid surface registration..." << std::endl;
        s_lock.lockForWrite();
        s_mask = nullptr;
        // If the respective FaceModel is already loaded by the FaceModelManager,
        // BackgroundModelLoader will just make a deep copy of the internal model
        // as it is right now.
        using BML = FileIO::BackgroundModelLoader;
        BML* loader = new BML( mpath.toStdString());
        connect( loader, &BML::loadedModel, [abspath](ObjModel::Ptr m)
        {
            s_mask = m; s_maskPath = abspath; s_lock.unlock();
            std::cerr << "Finished loading template mask for non-rigid surface registration" << std::endl;
        }); // end lambda
        connect( loader, &BML::finished, loader, &BML::deleteLater);
        loader->start();
    }   // end if

    return true;
}   // end setMaskPath


// static
bool ActionNonRigidRegistration::maskLoaded()
{
    if ( !s_lock.tryLockForRead())
        return false;
    const bool loaded = s_mask != nullptr;
    s_lock.unlock();
    return loaded;
}   // end maskLoaded


// static
QString ActionNonRigidRegistration::maskPath()
{
    QString mpath;
    s_lock.lockForRead();
    mpath = s_maskPath;
    s_lock.unlock();
    return mpath;
}   // end maskPath


// static
void ActionNonRigidRegistration::setKNN(int k)
{
    s_knn = std::max( 1, std::min( 99, k));
}   // end setKNN


namespace {

ObjModel::Ptr makeVertexModelFromAffinityMat( cv::SparseMat_<float>& A, const ObjModel& tgt)
{
    const size_t N = size_t(A.size(0));
#ifndef NDEBUG
    const int M = A.size(1);
    assert( tgt.numVtxs() == M);
    assert( tgt.hasSequentialVertexIds());
#endif

    // TODO normalize columns of A plus a small factor for noise tolerance.

    // Initialise the set of vertices that will define the point cloud to be returned.
    std::vector<cv::Vec3f> vtxs(N);
    std::vector<float> rowWts(N);  // Need to normalize the rows of A.
    for ( size_t i = 0; i < N; ++i)
    {
        vtxs[i] = cv::Vec3f(0,0,0);
        rowWts[i] = 0.0f;
    }   // end for

    // Do the virtual dot product of the affinity matrix row weights with the target mesh vertices.
    // Since this is a sparse matrix, only the non-zero entries are looked at.
    const cv::SparseMatConstIterator_<float> it_end = A.end();
    cv::SparseMatConstIterator_<float> it = A.begin();
    for ( ; it != it_end; ++it)
    {
        const int i = it.node()->idx[0];    // Vertex index of the template mask
        const int j = it.node()->idx[1];    // Vertex index of tgt
        const float w = it.value<float>();
        vtxs[size_t(i)] += w*tgt.vtx(j);
        rowWts[size_t(i)] += w;
    }   // end for

    ObjModel::Ptr nmod = ObjModel::create();
    for ( size_t i = 0; i < N; ++i)
        nmod->addVertex( vtxs[i] * 1.0f/rowWts[i]);

    return nmod;
}   // end makeVertexModelFromAffinityMat


void addConnectivity( const ObjModel& smod, ObjModel::Ptr nmod)
{
    assert( smod.hasSequentialFaceIds());
    assert( nmod->hasSequentialVertexIds());
    const int nf = smod.numPolys();
    for ( int fid = 0; fid < nf; ++fid)
    {
        const RFeatures::ObjPoly& f = smod.face(fid);
        nmod->addFace( f[0], f[1], f[2]);
    }   // end for
}   // end addConnectivity


void setTextureCoords( const FM* fm, ObjModel::Ptr nmod)
{
    assert( nmod->numMats() == 1);
    int sfid = -1;
    int kvidx;
    cv::Vec3f sfv;
    std::vector<cv::Vec2f> uvs( size_t(nmod->numVtxs()));
    const ObjModel& smod = fm->model();
    RFeatures::ObjModelSurfacePointFinder spfinder( smod);
    const int nv = nmod->numVtxs();
    for ( int vidx = 0; vidx < nv; ++vidx)
    {
        const cv::Vec3f& v = nmod->vtx(vidx);
        kvidx = fm->findVertex(v);
        spfinder.find( v, kvidx, sfid, sfv);
        uvs[size_t(vidx)] = smod.calcTextureCoords( sfid, sfv);
    }   // end for

    const int nf = nmod->numPolys();
    for ( int fid = 0; fid < nf; ++fid)
    {
        const RFeatures::ObjPoly& f = nmod->face(fid);
        nmod->setOrderedFaceUVs(0, fid, uvs[size_t(f[0])], uvs[size_t(f[1])], uvs[size_t(f[2])]);
    }   // end for
}   // end setTextureCoords
}   // end namespace


ActionNonRigidRegistration::ActionNonRigidRegistration( const QString& dn, const QIcon& ico) : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionNonRigidRegistration::checkEnable( Event) { return MS::isViewSelected();}


bool ActionNonRigidRegistration::doBeforeAction( Event)
{
    if ( !maskLoaded())
    {
        QWidget* prnt = static_cast<QWidget*>(parent());
        QMessageBox::information( prnt, tr("Template Mask Not Set!"), tr("First set a model as the template mask to use."));
        return false;
    }   // end if

    const FM* fm = MS::selectedModel();
    if ( !fm->model().hasSequentialVertexIds())
    {
        QWidget* prnt = static_cast<QWidget*>(parent());
        QMessageBox::information( prnt, tr("Invalid vertex ordering!"), tr("Model vertices must be ordered sequentially!"));
        return false;
    }   // emd if

    MS::showStatus( "Performing non-rigid surface registration against selected model...");
    return true;
}   // end doBeforeAction


void ActionNonRigidRegistration::doAction( Event)
{
    FM* fm = MS::selectedModel();

    // Make a deep copy of the mask which we will be transforming.
    s_lock.lockForRead();
    ObjModel::Ptr mask = s_mask->deepCopy();
    s_lock.unlock();

    RFeatures::ObjModelKNNCorresponder corresponder( *mask);

    fm->lockForRead();

    // Do the correspondence and return the affinity matrix.
    cv::SparseMat_<float> A = corresponder.sample( fm->kdtree(), s_knn);

    // Obtain the new model as weighted sums of the selected model's vertices.
    ObjModel::Ptr nmod = makeVertexModelFromAffinityMat( A, fm->model());

    if ( fm->model().numMats() > 0)
        nmod->copyInMaterials( fm->model());
    addConnectivity( *mask, nmod);
    if ( fm->model().numMats() > 0)
        setTextureCoords( fm, nmod);

    fm->unlock();

    fm->lockForWrite();
    fm->update( nmod);
    fm->moveLandmarksToSurface();
    fm->unlock();
}   // end doAction


void ActionNonRigidRegistration::doAfterAction( Event)
{
    MS::showStatus( "Finished non-rigid surface registration.", 5000);
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::LANDMARKS_CHANGE});
}   // end doAfterAction
