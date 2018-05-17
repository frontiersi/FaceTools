/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <FaceModelViewer.h>
#include <ObjModelHoleFiller.h>
#include <algorithm>
using FaceTools::Action::ActionFillHoles;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionFillHoles::ActionFillHoles( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico, true/*disable before other*/)
{
    addChangeTo( MODEL_GEOMETRY_CHANGED);
    addRespondTo( LANDMARK_ADDED);
    addRespondTo( LANDMARK_DELETED);
    addRespondTo( LANDMARK_CHANGED);
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionFillHoles::doAction( FaceControlSet& rset)
{
    assert(rset.size() == 1);
    FaceControl* fc = rset.first();
    FaceModel* fm = fc->data();

    const RFeatures::ObjModelInfo& info = fm->info();
    RFeatures::ObjModelHoleFiller hfiller( fm->model());
    int nc = (int)info.components().size();
    for ( int c = 0; c < nc; ++c)
    {
        const IntSet* bidxs = info.components().cboundaries(c);
        if ( bidxs == NULL) // Cannot fill holes on a component without boundaries
            continue;

        IntSet hbs = *bidxs;   // Copy out the boundary indices for the component
        hbs.erase( info.components().lboundary(c));    // Erase the longest boundary (which is very likely the outer boundary)

        for ( int i : hbs)
        {
            const std::list<int>& blist = info.boundaries().boundary(i);
            IntSet newPolys;
            hfiller.fillHole( blist, &newPolys);
            std::cerr << " Filled hole (boundary " << i << ") on component " << c << " with " << newPolys.size() << " polygons" << std::endl;
        }   // end for
    }   // end for

    fm->updateData(fm->model());  // Destructive update
    return true;
}   // end doAction
