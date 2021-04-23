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

#include <Action/ActionExtractFace.h>
#include <Action/ActionAlignModel.h>
#include <Interactor/RadialSelectHandler.h>
#include <FaceModel.h>
#include <r3d/Copier.h>
using FaceTools::Action::ActionExtractFace;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::RadialSelectHandler;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;


float ActionExtractFace::s_cropRadius(110.0f);
void ActionExtractFace::setCropRadius( float d) { s_cropRadius = std::max( 1.0f, d);}
float ActionExtractFace::cropRadius() { return s_cropRadius;}


ActionExtractFace::ActionExtractFace( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _ev(Event::NONE)
{
    // Required to change action name when activating radial select
    addRefreshEvent( Event::VIEW_CHANGE);
    setAsync(true);
}   // end ctor


QString ActionExtractFace::toolTip() const
{
    return "Extract a manually demarcated region, or the identified facial region.";
}   // end toolTip


QString ActionExtractFace::whatsThis() const
{
    QStringList htxt;
    htxt << "With the radial selection tool active, this function discards parts of the model outside";
    htxt << "of the selected boundary. If not using the radial selection tool, the facial region";
    htxt << "is identified automatically and parts outside of it discarded.";
    htxt << "Automatic face detection works best for models with texture. For untextured models,";
    htxt << "a simple 'front-finding' algorithm heuristically estimates the position of";
    htxt << "the nose tip and defines a region centred at that point with fixed radius.";
    htxt << "If landmarks are defined, extraction retains the portion of the face within";
    htxt << "the radial distance given by the most distant landmark from the mean position";
    htxt << "of the medial landmarks.";
    return htxt.join( " ");
}   // end whatsThis


bool ActionExtractFace::update( Event e)
{
    const RadialSelectHandler *h = MS::handler<RadialSelectHandler>();
    const QString actName = h && h->isEnabled() ? "Extract Region" : "Extract Face";
    setDisplayName( actName);
    return true;
}   // end update


bool ActionExtractFace::isAllowed( Event) { return MS::isViewSelected();}


bool ActionExtractFace::doBeforeAction( Event)
{
    const RadialSelectHandler *h = MS::handler<RadialSelectHandler>();
    if ( h->isEnabled())
        MS::showStatus("Extracting region...");
    else
        MS::showStatus("Extracting face...");
    _ev = Event::MESH_CHANGE | Event::AFFINE_CHANGE;
    if ( MS::selectedModelScopedRead()->hasLandmarks())
        _ev |= Event::LANDMARKS_CHANGE;
    storeUndo( this, _ev);
    return true;
}   // end doBeforeAction


namespace {
r3d::Mesh::Ptr cropRegion( const r3d::Mesh &mesh, const IntSet &cfids)
{
    // Copy the subset of faces into a new model
    r3d::Mesh::Ptr nmod;
    if ( !cfids.empty())
    {
        const r3d::Mat4f T = mesh.transformMatrix();
        r3d::Copier copier( mesh);
        std::for_each( std::begin(cfids), std::end(cfids), [&](int fid){ copier.add(fid);});
        nmod = copier.copiedMesh();
        nmod->setTransformMatrix( T);
    }   // end if
    return nmod;
}   // end cropRegion


r3d::Mesh::Ptr extractFacialRegion( const FM &fm, r3d::Vec3f pos, float sqd)
{
    std::vector<std::pair<size_t, float> > vtxs;
    fm.kdtree().findr( pos, sqd, vtxs);  // Points within d cm of the given position
    IntSet vidxs;
    for ( const auto &p : vtxs)
        vidxs.insert(int(p.first));
    r3d::Mesh::Ptr face = fm.mesh().extractVerticesSubset( vidxs, 0, true);
    face->copyInMaterials( fm.mesh());
    return face;
}   // end extractFacialRegion
}   // end namespace


r3d::Mesh::Ptr ActionExtractFace::extract( FM &fm)
{
    float sqrad = 0.0f;
    Vec3f pos = Vec3f::Zero();

    // Calculate the maximum squared radius and the mean position from all assessments
    int count = 0;
    for ( int aid : fm.assessmentIds())
    {
        const Landmark::LandmarkSet &lmks = fm.assessment( aid)->landmarks();
        if ( !lmks.empty())
        {
            sqrad = std::max( sqrad, lmks.sqRadius());
            pos += lmks.medialMean();
            count++;
        }   // end if
    }   // end for

    if ( count == 0)
    {
        sqrad = cropRadius() * cropRadius();
        ActionAlignModel::align( fm);
        fm.fixTransformMatrix();
    }   // end else
    else
        pos /= count;

    return extractFacialRegion( fm, pos, sqrad);
}   // end extract


void ActionExtractFace::doAction( Event)
{
    FM::WPtr fm = MS::selectedModelScopedWrite();
    r3d::Mesh::Ptr nmod;

    RadialSelectHandler *handler = MS::handler<RadialSelectHandler>();
    if ( handler->isEnabled())
        nmod = cropRegion( fm->mesh(), handler->selectedFaces());
    else
        nmod = extract( *fm);

    if ( nmod && nmod->numFaces() < fm->mesh().numFaces())
        fm->update( nmod, true, true, 1);    // Keep just one manifold
    else
        _ev = Event::NONE;
}   // end doAction


Event ActionExtractFace::doAfterAction( Event)
{
    if ( _ev == Event::NONE)
    {
        scrapLastUndo( MS::selectedModel());
        MS::showStatus("Model unchanged.", 5000);
    }   // end if
    else
    {
        RadialSelectHandler *handler = MS::handler<RadialSelectHandler>();
        if ( handler)
            handler->reset( MS::selectedModel());
        MS::showStatus("Extracted region.", 5000);
    }   // end else
    return _ev;
}   // end doAfterAction
