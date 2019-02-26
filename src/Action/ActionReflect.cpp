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

#include <ActionReflect.h>
#include <ObjModelReflector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionReflect;
using FaceTools::Action::EventSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Path;
using FaceTools::PathSet;
using FaceTools::Landmark::LandmarkSet;


ActionReflect::ActionReflect( const QString& dn, const QIcon& ico, QProgressBar* pb)
    : FaceAction(dn, ico)
{
    if ( pb)
        setAsync(true, QTools::QProgressUpdater::create(pb));
}   // end ctor


bool ActionReflect::testReady( const FV* fv) { return !fv->data()->landmarks()->empty();}


namespace {

void reflectPaths( PathSet::Ptr paths, const cv::Vec3f& ppt, const cv::Vec3f& pvec)
{
    for ( int pid : paths->ids())
    {
        Path* path = paths->path(pid);
        RFeatures::ObjModelReflector::reflectPoint( *path->vtxs.begin(), ppt, pvec);
        RFeatures::ObjModelReflector::reflectPoint( *path->vtxs.rbegin(), ppt, pvec);
    }   // end for
}   // end reflectPaths


void reflectLandmarks( LandmarkSet::Ptr lmks, const cv::Vec3f& ppt, const cv::Vec3f& pvec)
{
    using namespace FaceTools;
    for ( int id : lmks->ids())
    {
        if ( LDMKS_MAN::landmark(id)->isBilateral())
        {
            cv::Vec3f p0 = *lmks->pos(id, FACE_LATERAL_LEFT);
            cv::Vec3f p1 = *lmks->pos(id, FACE_LATERAL_RIGHT);
            RFeatures::ObjModelReflector::reflectPoint( p0, ppt, pvec);
            RFeatures::ObjModelReflector::reflectPoint( p1, ppt, pvec);
            lmks->set( id, p0, FACE_LATERAL_LEFT);
            lmks->set( id, p1, FACE_LATERAL_RIGHT);
        }   // end if
        else
        {
            cv::Vec3f p = *lmks->pos(id);
            RFeatures::ObjModelReflector::reflectPoint( p, ppt, pvec);
            lmks->set(id, p);
        }   // end else
    }   // end for
}   // end reflectLandmarks

}   // end namespace


bool ActionReflect::doAction( FVS& fvs, const QPoint&)
{
    assert(fvs.size() == 1);
    FV* fv = fvs.first();
    FM* fm = fv->data();

    fm->lockForWrite();

    const RFeatures::Orientation& on = fm->landmarks()->orientation();
    RFeatures::ObjModelInfo::Ptr info = fm->info();
    RFeatures::ObjModel::Ptr model = info->model();

    cv::Vec3f pvec;
    cv::normalize( on.uvec().cross( on.nvec()), pvec);    // Reflection plane vector (normalized)
    const cv::Vec3f ppt = fm->landmarks()->fullMean();    // Point in reflection plane

    // Reflect the underlying model
    RFeatures::ObjModelReflector reflector( model);
    reflector.reflect( ppt, pvec);

    // Also need to reflect landmarks and paths before updating FaceModel
    reflectLandmarks( fm->landmarks(), ppt, pvec);
    reflectPaths( fm->paths(), ppt, pvec);

    info->reset( model);
    fm->update(info);

    fm->unlock();
    return true;
}   // end doAction


void ActionReflect::doAfterAction( EventSet& es, const FVS& fvs, bool)
{
    es.insert(GEOMETRY_CHANGE);

    assert(fvs.size() == 1);
    FV* fv = fvs.first();
    FM* fm = fv->data();
    if ( !fm->landmarks()->empty())
        es.insert(LANDMARKS_CHANGE);
    if ( !fm->paths()->empty())
        es.insert(METRICS_CHANGE);
}   // end doAfterAction
