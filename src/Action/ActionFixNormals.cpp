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

#include <Action/ActionFixNormals.h>
#include <QMessageBox>
#include <FaceModel.h>
#include <r3d/FaceParser.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionFixNormals;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ActionFixNormals::ActionFixNormals( const QString& dn, const QIcon& ico) : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionFixNormals::isAllowed( Event) { return MS::isViewSelected();}


bool ActionFixNormals::doBeforeAction( Event)
{
    MS::showStatus( "Fixing inconsistent normals on selected model...");
    storeUndo( this, Event::MESH_CHANGE);
    return true;
}   // end doBeforeAction


namespace {

struct NormalAgreementParser : r3d::TriangleParser
{
    void parseTriangle( int fid, int vroot, int va, int vb) override
    {
        const r3d::Vec3f &v0 = mesh->uvtx(vroot);
        const r3d::Vec3f &v1 = mesh->uvtx(va);
        const r3d::Vec3f &v2 = mesh->uvtx(vb);
        const r3d::Vec3f e10 = v1 - v0;
        const r3d::Vec3f e20 = v2 - v0;

        const r3d::Vec3f cvec = e20.cross(e10);
        const r3d::Vec3f mvec = mesh->calcFaceVector( fid);
        // If cvec is in the same direction as mvec (i.e. according to the triangle's current stored vertex order),
        // then the face id goes in the agree vector, otherwise it goes in disagree.
        // Note that we don't know what the CORRECT ordering is at this stage since we cannot specify an
        // authoritative ordering of edges e10 and e20 for the cross product above. But the expectation is
        // that the set of triangles with vertex orderings that we want to change is the smaller of the
        // two sets when parsing completes (see getInconsistent below).
        if ( mvec.dot(cvec) > 0)
            _agreeFaces.push_back(fid);
        else
            _disagreeFaces.push_back(fid);
    }   // end parseTriangle

    // The "inconsistent" faces is the set that is smaller since we don't know which side
    // was the "correct" side to start on when we initiated the parsing of faces on this manifold.
    const std::vector<int> &getInconsistent() const
    {
        const std::vector<int> *efids = &_agreeFaces;
        if ( _disagreeFaces.size() < _agreeFaces.size())
            efids = &_disagreeFaces;
        return *efids;
    }   // end getInconsistent

private:
    // Vectors that will hold the ids of the faces with normals calculated from their stored
    // indices that either do or don't agree with the normals calculated using the obtained
    // vertex order from r3d::FaceParser.
    std::vector<int> _agreeFaces;
    std::vector<int> _disagreeFaces;
};  // end struct


struct ManifoldBoundaryParser : r3d::BoundaryParser
{
    ManifoldBoundaryParser( const r3d::Manifold &m) : _manf(m)
    {
#ifndef NDEBUG
        std::cerr << "Manifold edge count: " << _manf.edges().size() << std::endl;
        std::cerr << "Manifold face count: " << _manf.faces().size() << std::endl;
        std::cerr << "Manifold vtxs count: " << _manf.vertices().size() << std::endl;
        std::cerr << "Manifold bnds count: " << _manf.boundaries().count() << std::endl;
#endif
    }   // end ctor

    // Only go beyond this edge if e is not a boundary edge on this (or another) manifold.
    bool parseEdge( int fid, const r3d::Vec2i &e, int&) override
    {
        assert( _manf.faces().count( fid) > 0);
        const r3d::Mesh &mesh = _manf.mesh();
        const int eid = mesh.edgeId(e);
        return _manf.edges().count(eid) == 0 && mesh.nsfaces(eid) <= 2;
    }   // end parseEdge

private:
    const r3d::Manifold &_manf;
};  // end struct

}   // end namespace


int ActionFixNormals::fixNormals( FM *fm)
{
    const r3d::Mesh& mesh = fm->mesh();
    const r3d::Manifolds &manfs = fm->manifolds();
    bool twisted = false;
    IntSet efids;

    // Parse each manifold in turn finding which faces agree and which don't.
    const int nm = int(manfs.count());
    for ( int i = 0; i < nm; ++i)
    {
        r3d::FaceParser fparser( mesh);

        NormalAgreementParser naparser;
        fparser.addTriangleParser( &naparser);

        ManifoldBoundaryParser boundaryParser( manfs[i]);
        fparser.setBoundaryParser( &boundaryParser);

        const int sfid = *manfs[i].faces().begin();
        fparser.parse( sfid);
        // Only reverse the order if twisted returned false
        if ( !fparser.twisted())
        {
            const std::vector<int> &fids = naparser.getInconsistent();
            efids.insert( fids.begin(), fids.end());
        }   // end if
        else
        {
            std::cerr << "[INFO] FaceTools::ActionFixNormals::fixNormals: Manifold " << i << " has a twisted surface" << std::endl;
            twisted = true;
        }   // end else
    }   // end for

    if ( !efids.empty())
    {
        r3d::Mesh::Ptr mesh = fm->mesh().deepCopy();
        for ( int fid : efids)
            mesh->reverseFaceVertices(fid);
        fm->update( mesh, false, false);
    }   // end if

    return twisted ? -1 * int(efids.size()) : int(efids.size());
}   // end fixNormals


void ActionFixNormals::doAction( Event)
{
    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    _nfixed = fixNormals( fm);
    fm->unlock();
}   // end doAction


Event ActionFixNormals::doAfterAction( Event)
{
    QString msg;
    if ( _nfixed < 0)
        msg = tr("Fixed %1 normal inconsistencies (but at least one manifold is twisted onto itself).").arg( abs(_nfixed));
    else if ( _nfixed > 0)
        msg = tr("Fixed %1 normal inconsistencies.").arg( abs(_nfixed));
    else
    {
        scrapLastUndo( MS::selectedModel());
        msg = tr("Finished checking normals - found no inconsistencies.");
    }   // end else

    MS::showStatus( msg, 5000);

    if ( _nfixed < 0)
    {
        // Print warning about being a twisted mesh
        const QString msg = tr("Normals cannot be consistently ordered due to at least one manifold being twisted onto itself.");
        QMB::warning( static_cast<QWidget*>(parent()), tr("Normal Fixing Failed!"),
                            QString("<p align='center'>%1</p>").arg(msg));
    }   // end if

    return _nfixed != 0 ? Event::MESH_CHANGE : Event::NONE;
}   // end doAfterAction

