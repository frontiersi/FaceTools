/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <FaceModel.h>
#include <FaceView.h>
#include <cassert>
using FaceTools::Action::ActionBackfaceCulling;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FMVS;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMV;


ActionBackfaceCulling::ActionBackfaceCulling( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable( true, false);
}   // end ctor


bool ActionBackfaceCulling::testReady( const FV* fv)
{
    setChecked( fv->backfaceCulling());
    return true;
}   // end testReady


bool ActionBackfaceCulling::doAction( FVS& fvs, const QPoint&)
{
    // Apply to all FaceViews in all directly selected viewers.
    const bool ischecked = isChecked();
    FMVS fmvs = fvs.dviewers();
    fvs.clear();
    for ( const FMV* v : fmvs)
    {
        for ( FV* f : v->attached())
        {
            f->setBackfaceCulling( ischecked);
            fvs.insert(f);
        }   // end for
    }   // end for
    return true;
}   // end doAction
