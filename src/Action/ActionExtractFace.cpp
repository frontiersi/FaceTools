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

#include <Action/ActionExtractFace.h>
#include <Detect/FaceAlignmentFinder.h>
#include <FaceModel.h>
#include <QMessageBox>
#include <r3d/Copier.h>
#include <r3d/FrontFinder.h>
#include <r3d/VectorPCFinder.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionExtractFace;
using FaceTools::Action::Event;
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


float ActionExtractFace::s_cropRadius(100.0f);
void ActionExtractFace::setCropRadius( float d) { s_cropRadius = std::max( 1.0f, d);}
float ActionExtractFace::cropRadius() { return s_cropRadius;}


ActionExtractFace::ActionExtractFace( const QString& dn, const QIcon& ico, const RadialSelectHandler &handler)
    : FaceAction(dn, ico), _handler(handler)
{
    setAsync(true);
}   // end ctor


QString ActionExtractFace::toolTip() const
{
    return "Extract a demarcated region, or the automatically identified facial region.";
}   // end toolTip


QString ActionExtractFace::whatsThis() const
{
    QStringList htxt;
    htxt << "With the radial selection tool active, this function will crop out the selected";
    htxt << "region to be retained. If the radial selection tool is not active, the facial region";
    htxt << "will be identified automatically and this will be cropped for retaining. Note that";
    htxt << "automatic face finding works best for models with texture. For untextured models, a";
    htxt << "simple 'front-finding' algorithm is used to heuristically estimate the position of";
    htxt << "the nose tip and to then define a region centred at that point having fixed radius.";
    htxt << "If landmarks are defined, extraction retains the portion of the face within";
    htxt << "the radial distance given by the most distant landmark from the mean position";
    htxt << "of the medial landmarks.";
    return htxt.join( " ");
}   // end whatsThis


bool ActionExtractFace::isAllowed( Event) { return MS::isViewSelected();}


bool ActionExtractFace::doBeforeAction( Event)
{
    if ( MS::selectedModel()->hasLandmarks())
    {
        static const QString msg = tr("Landmark positions may be perturbed; continue?");
        if ( QMessageBox::warning( static_cast<QWidget*>(parent()),
                    tr("Landmarks present!"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return false;
    }   // end if

    if ( _handler.isEnabled())
        MS::showStatus("Extracting radially bounded region...");
    else
        MS::showStatus("Extracting facial region...");

    _nmod = nullptr;
    return true;
}   // end doBeforeAction


void ActionExtractFace::doAction( Event)
{
    FM *fm = MS::selectedModel();
    fm->lockForRead();

    r3d::Mesh::Ptr nmod;
    if ( _handler.isEnabled())
        nmod = cropRegion( fm, _handler.selectedFaces());
    else
    {
        float crad = cropRadius();
        if ( fm->hasLandmarks())
            crad = sqrtf( fm->currentLandmarks().sqRadius());
        nmod = extractFacialRegion( fm, crad);
    }   // end else

    if ( nmod->numFaces() < fm->mesh().numFaces())
        _nmod = nmod;
    fm->unlock();
}   // end doAction


Event ActionExtractFace::doAfterAction( Event)
{
    Event e = Event::NONE;
    if ( !_nmod)
        MS::showStatus("Model was unchanged.", 5000);
    else
    {
        FM *fm = MS::selectedModel();
        fm->lockForWrite();

        e |= Event::MESH_CHANGE | Event::CONNECTIVITY_CHANGE;
        if ( fm->hasLandmarks())
            e |= Event::LANDMARKS_CHANGE;
        storeUndo( this, e);

        fm->update( _nmod, true, true, 1);    // Keep just one manifold
        fm->unlock();

        MS::showStatus("Extracted facial region.", 5000);
    }   // end else
    _nmod = nullptr;
    return e;
}   // end doAfterAction


r3d::Mesh::Ptr ActionExtractFace::extractFacialRegion( const FM *fm, float d, r3d::Mat4f* iT)
{
    Mat4f T = Mat4f::Zero();
    if ( fm->hasMask())
        T = fm->transformMatrix();
    else if ( fm->hasTexture())
        T = Detect::FaceAlignmentFinder( fm->kdtree(), 650.0f).find( fm->bounds()[0]->centre());

    bool estRotMat = false;
    if ( T.isZero())
    {
        T = Mat4f::Identity();
        if ( fm->hasTexture())
            std::cerr << "Face detection/alignment using texture failed - estimating from geometry..." << std::endl;
        r3d::FrontFinder fff( fm->mesh());   // Geometrically estimate
        T.block<3,1>(0,3) = fff();
        estRotMat = true;
    }   // end else

    std::vector<std::pair<size_t, float> > vtxs;
    fm->kdtree().findr( T.block<3,1>(0,3), powf(d,2), vtxs);  // Points within d cm of the found position
    IntSet vidxs;
    for ( const auto &p : vtxs)
        vidxs.insert(int(p.first));

    if ( iT && estRotMat)
        T.block<3,3>(0,0) = r3d::VectorPCFinder::estimateRotationMatrix( fm->mesh(), vidxs);

    if ( iT)
        *iT = T;

    r3d::Mesh::Ptr face = fm->mesh().extractVerticesSubset( vidxs, 0, true);
    face->copyInMaterials( fm->mesh());

    return face;
}   // end extractFacialRegion


r3d::Mesh::Ptr ActionExtractFace::cropRegion( const FM *fm, const IntSet &cfids)
{
    // Copy the subset of faces into a new model
    r3d::Mesh::Ptr nmod;
    if ( !cfids.empty())
    {
        const r3d::Mat4f T = fm->mesh().transformMatrix();
        r3d::Copier copier( fm->mesh());
        std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.add(fid);});
        nmod = copier.copiedMesh();
        nmod->setTransformMatrix( T);
    }   // end if
    return nmod;
}   // end cropRegion

