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

#include <ActionBackfaceCulling.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionBackfaceCulling;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceViewerSet;
using FaceTools::FaceControl;


ActionBackfaceCulling::ActionBackfaceCulling( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
}   // end ctor


bool ActionBackfaceCulling::testReady( const FaceControl* fc)
{
    setChecked( fc->view()->backfaceCulling());
    return true;
}   // end testReady


bool ActionBackfaceCulling::doAction( FaceControlSet& fcs, const QPoint&)
{
    // Apply to all FaceControls in all directly selected viewers.
    FaceViewerSet fvs = fcs.directViewers();
    fcs.clear();
    for ( const FaceModelViewer* v : fvs)
    {
        for ( FaceControl* f : v->attached())
        {
            f->view()->setBackfaceCulling( isChecked());
            fcs.insert(f);
        }   // end for
    }   // end for
    return true;
}   // end doAction
