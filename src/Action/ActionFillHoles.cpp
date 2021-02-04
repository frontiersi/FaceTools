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

#include <Action/ActionFillHoles.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <r3d/HoleFiller.h>
#include <algorithm>
using FaceTools::Action::ActionFillHoles;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ActionFillHoles::ActionFillHoles( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    addRefreshEvent( Event::MESH_CHANGE);
    setAsync(true);
}   // end ctor


bool ActionFillHoles::isAllowed( Event)
{
    const FM *fm = MS::selectedModel();
    if ( !fm)
        return false;

    fm->lockForRead();
    // Ready if there's more than one boundary on any component.
    bool rval = false;
    const int nc = static_cast<int>(fm->manifolds().count());
    for ( int c = 0; c < nc; ++c)
    {
        if ( fm->manifolds()[c].boundaries().count() > 1)
        {
            rval = true;
            break;
        }   // end if
    }   // end for
    fm->unlock();
    return rval;
}   // end isAllowed


bool ActionFillHoles::doBeforeAction( Event)
{
    MS::showStatus( "Filling model holes...");
    storeUndo( this, Event::MESH_CHANGE);
    return true;
}   // end doBeforeAction


namespace {
size_t getNumHoles( const r3d::Manifolds& manf)
{
    int nholes = 0;
    using namespace r3d;
    const size_t nm = manf.count();
    for ( size_t i = 0; i < nm; ++i)
    {
        const r3d::Manifold& man = manf[int(i)];
        const r3d::Boundaries& bnds = man.boundaries();
        const int nbs = static_cast<int>(bnds.count()); // Can be zero
        nholes += std::max( 0, nbs - 1);
    }   // end for
    return static_cast<size_t>(nholes);
}   // end getNumHoles

}   // end namespace


void ActionFillHoles::doAction( Event)
{
    using namespace r3d;

    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    const Manifolds* manfs = &fm->manifolds();
    const size_t nm = manfs->count();
    Mesh::Ptr mesh = fm->mesh().deepCopy();
    Manifolds::Ptr nmanfs;

    while ( true)
    {
        HoleFiller hfiller( mesh);
        std::vector<int> mholes(nm);    // Record the number of holes per manifold

        int sumPolysAdded = 0;    // Total polygons added
        for ( size_t i = 0; i < nm; ++i)
        {
            const r3d::Manifold& man = manfs->at(int(i));
            const IntSet& mpolys = man.faces();
            assert( !mpolys.empty());

            const Boundaries& bnds = man.boundaries();
            const int nbs = static_cast<int>(bnds.count()); // Can be zero
            mholes[i] = std::max(0, nbs-1);

            int polysAdded = 0;
            for ( int j = 1; j < nbs; ++j)  // Ignore the first (longest) boundary
            {
                const std::list<int>& blist = bnds.boundary(j);
                polysAdded += hfiller.fillHole( blist, mpolys);
            }   // end for
#ifndef NDEBUG
            if ( nbs > 1)
            {
                std::cerr << "Manifold " << i << ": " << std::setw(4) << mholes[i]
                          << " holes filled with " << std::setw(4) << polysAdded << " polygons" << std::endl;
            }   // end if
#endif
            sumPolysAdded += polysAdded;
        }   // end for

        // If no polygons added, break loop.
        if ( sumPolysAdded == 0)
            break;

        nmanfs = Manifolds::create( *mesh);
        for ( size_t i = 0; i < nm; ++i)
            nmanfs->at(int(i)).boundaries();  // Causes boundary edges to be calculated
        manfs = nmanfs.get();
    }   // end while

    fm->update( mesh, true, true);
    fm->unlock();
}   // end doAction


Event ActionFillHoles::doAfterAction( Event)
{
    const size_t nh = getNumHoles( MS::selectedModel()->manifolds());
    MS::showStatus( QString("Finished hole filling; %1 hole%2 remain%3.").arg(nh == 0 ? "no" : QString("%1").arg(nh)).arg( nh != 1 ? "s" : "").arg( nh == 1 ? "s" : ""), 5000);
    return Event::MESH_CHANGE;
}   // end doAfterAction

