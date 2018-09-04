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

#include <ActionSmooth.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <ObjModelSmoother.h>   // RFeatures
#include <FaceModelSurfaceData.h>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionSmooth;
using FaceTools::Action::EventSet;
using FaceTools::FaceModelSurfaceData;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;


ActionSmooth::ActionSmooth( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico), _maxc(0.8)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionSmooth::testReady( const FV* fv)
{
    return FaceModelSurfaceData::get()->isAvailable(fv->data());
}   // end testReady


bool ActionSmooth::doAction( FVS& rset, const QPoint&)
{
    assert(rset.size() == 1);
    FV* fv = rset.first();
    FaceModel* fm = fv->data();
    SurfaceData::WPtr msd = FaceModelSurfaceData::wdata(fm);    // Note here that a write lock on the model is provided
    RFeatures::ObjModelInfo::Ptr info = fm->info();
    RFeatures::ObjModelSmoother smoother( info->model(), msd->curvature, &msd->normals, &msd->pareas);
    smoother.smooth( maxCurvature());
    bool success = true;
    if ( info->reset())
        fm->update( info);  // Destructive update
    else
    {
        std::cerr << "[ERROR] FaceTools::ActionSmooth::doAction: Failed to update model info post smooth!" << std::endl;
        success = false;
    }   // end else
    return success;
}   // end doAction


// protected
void ActionSmooth::doAfterAction( EventSet& cs, const FVS&, bool)
{ 
    cs.insert( GEOMETRY_CHANGE);
    cs.insert( SURFACE_DATA_CHANGE);
}   // end doAfterAction
