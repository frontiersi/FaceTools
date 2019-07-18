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

#include <ActionFillHoles.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelHoleFiller.h>
#include <algorithm>
using FaceTools::Action::ActionFillHoles;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionFillHoles::ActionFillHoles( const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico)
{
    setAsync(true);
}   // end ctor


bool ActionFillHoles::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    if ( !fm)
        return false;

    fm->lockForRead();
    // Ready if there's more than one boundary on any component.
    bool rval = false;
    const int nc = static_cast<int>(fm->manifolds().count());
    for ( int c = 0; c < nc; ++c)
    {
        if ( fm->manifolds().manifold(c)->boundaries( fm->model()).count() > 1)
        {
            rval = true;
            break;
        }   // end if
    }   // end for
    fm->unlock();
    return rval;
}   // end testReady


bool ActionFillHoles::doBeforeAction( Event)
{
    MS::showStatus( "Filling model holes...");
    return true;
}   // end doBeforeAction


namespace {
size_t getNumHoles( const RFeatures::ObjModel& model, const RFeatures::ObjModelManifolds& manf)
{
    int nholes = 0;
    using namespace RFeatures;
    const size_t nm = manf.count();
    for ( size_t i = 0; i < nm; ++i)
    {
        const ObjManifold& man = *manf.manifold(int(i));
        const ObjModelManifoldBoundaries& bnds = man.boundaries(model);
        const int nbs = static_cast<int>(bnds.count()); // Can be zero
        nholes += std::max( 0, nbs - 1);
    }   // end for
    return static_cast<size_t>(nholes);
}   // end getNumHoles


bool wasHoleFilled( const RFeatures::ObjModel& model, const RFeatures::ObjModelManifolds& manf, const std::vector<int>& mholes)
{
    using namespace RFeatures;
    const size_t nm = manf.count();
    for ( size_t i = 0; i < nm; ++i)
    {
        const ObjModelManifoldBoundaries& bnds = manf.manifold(int(i))->boundaries(model);  // Causes boundary edges to be calculated
        const int nholes = std::max(0, static_cast<int>( bnds.count())-1);
        if ( mholes[i] > nholes)    // A hole was filled
            return true;
    }   // end for
    return false;
}   // end wasHoleFilled

}   // end namespace


void ActionFillHoles::doAction( Event)
{
    storeUndo( this, {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE});

    using namespace RFeatures;

    FM* fm = MS::selectedModel();
    fm->lockForWrite();
    const ObjModelManifolds* manfs = &fm->manifolds();
    const size_t nm = manfs->count();
    ObjModel::Ptr model = fm->wmodel();

    ObjModelManifolds::Ptr nmanfs;
    bool fillingHoles = true;
    while ( fillingHoles)
    {
        ObjModelHoleFiller hfiller( model);

        std::vector<int> mholes(nm);    // Record the number of holes per manifold
        for ( size_t i = 0; i < nm; ++i)
        {
            const ObjManifold* man = manfs->manifold(int(i));
            const IntSet& mpolys = man->polygons();
            assert( !mpolys.empty());

            const ObjModelManifoldBoundaries& bnds = man->boundaries(*model);
            const int nbs = static_cast<int>(bnds.count()); // Can be zero
            mholes[i] = std::max(0, nbs-1);

            int fh = 0;
            for ( int j = 1; j < nbs; ++j)  // Ignore the first (longest) boundary
            {
                const std::list<int>& blist = bnds.boundary(j);
                fh += hfiller.fillHole( blist, mpolys);
            }   // end for

            if ( nbs > 1)
            {
                std::cerr << "Manifold " << i << ": " << std::setw(4) << mholes[i]
                          << " holes filled with " << std::setw(4) << fh << " polygons" << std::endl;
            }   // end if
        }   // end for

        // Reanalyse the manifolds. If no change in number of holes in all manifolds, break loop.
        nmanfs = ObjModelManifolds::create( *model);
        manfs = nmanfs.get();
        fillingHoles = wasHoleFilled( *model, *manfs, mholes);
    }   // end while

    fm->update( model);
    fm->unlock();
}   // end doAction


void ActionFillHoles::doAfterAction( Event)
{
    const FM* fm = MS::selectedModel();

    const size_t nh = getNumHoles( fm->model(), fm->manifolds());
    MS::showStatus( QString("Finished hole filling; %1 hole%2 remain%3.").arg(nh == 0 ? "no" : QString("%1").arg(nh)).arg( nh != 1 ? "s" : "").arg( nh == 1 ? "s" : ""), 5000);
    emit onEvent( {Event::GEOMETRY_CHANGE, Event::CONNECTIVITY_CHANGE});
}   // end doAfterAction

