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

#include <ActionMapSurfaceData.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <ObjModelTriangleMeshParser.h>     // RFeatures
#include <algorithm>
using FaceTools::Action::ActionMapSurfaceData;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using RFeatures::ObjModelCurvatureMetrics;
using RFeatures::ObjModelCurvatureMap;
using RFeatures::ObjModelNormals;
using RFeatures::ObjModelPolygonAreas;
using RFeatures::ObjModelTriangleMeshParser;
using RFeatures::ObjModelInfo;
using RFeatures::ObjModel;


namespace {

int findMaxZPolygon( const ObjModel* model, const IntSet& vidxs)
{
    int zp = -1;
    float zval = -FLT_MAX;
    for ( int vid: vidxs)
    {
        const float zt = model->vtx(vid)[2];
        if ( zt > zval)
        {
            zp = vid;
            zval = zt;
        }   // end if
    }   // end for
    return *model->getFaceIds(zp).begin();
}   // end findMaxZPolygon

}   // end namespace


struct ActionMapSurfaceData::SurfaceData
{
    SurfaceData( const ObjModelInfo& info)
    {
        int nc = (int)info.components().size();   // # components

        // Parse all the components in the model to extract normals and polygon areas
        const ObjModel* model = info.model().get();
        IntSet pfset;   // For tracking the parsed faces
        ObjModelTriangleMeshParser parser(model);
        parser.addTriangleParser( &normals);
        parser.addTriangleParser( &pareas);
        for ( int c = 0; c < nc; ++c)
        {
            int zp = findMaxZPolygon( model, *info.components().componentVertices(c));
            IntSet pset;
            parser.setParseSet( &pset);
            parser.parse( zp, cv::Vec3d(0,0,1));
            pfset.insert( pset.begin(), pset.end());
        }   // end for

        // Now map the curvature to the surface of each of the components
        curvature = ObjModelCurvatureMap::create( model, &normals, &pareas);
        for ( int c = 0; c < nc; ++c)
            curvature->map( *info.components().componentVertices(c));

        // Create the different metrics
        metrics = new ObjModelCurvatureMetrics( curvature.get());
    }   // end ctor

    ~SurfaceData()
    {
        delete metrics;
    }   // end dtor

    ObjModelCurvatureMap::Ptr curvature;
    ObjModelCurvatureMetrics *metrics;
    ObjModelPolygonAreas pareas;
    ObjModelNormals normals;
};  // end struct


ActionMapSurfaceData::ActionMapSurfaceData( const QString& dname, const QIcon& icon, QProgressBar* pb)
    : FaceAction( dname, icon, true/*disable before other*/)
{
    addChangeTo( SURFACE_METRICS_CALCULATED);
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    if ( pb)
        setAsync( true, QTools::QProgressUpdater::create(pb));
}   // end ctor


ActionMapSurfaceData::~ActionMapSurfaceData()
{
    std::for_each( std::begin(_cmaps), std::end(_cmaps), [](auto p){ delete p.second;});
    _cmaps.clear();
}   // end dtor


const RFeatures::ObjModelCurvatureMetrics& ActionMapSurfaceData::metrics( const FaceControl* fc)
{
    ensureProcessed(fc);
    return *_cmaps.at(fc->data())->metrics;
}   // end metrics


const RFeatures::ObjModelCurvatureMap& ActionMapSurfaceData::curvature( const FaceControl* fc)
{
    ensureProcessed(fc);
    return *_cmaps.at(fc->data())->curvature.get();
}   // end curvature


const RFeatures::ObjModelNormals& ActionMapSurfaceData::normals( const FaceControl* fc)
{
    ensureProcessed(fc);
    return _cmaps.at(fc->data())->normals;
}   // end normals


// private
void ActionMapSurfaceData::ensureProcessed( const FaceControl* fc)
{
    const FaceModel* fm = fc->data();
    if ( _cmaps.count(fm) == 0)
        process( const_cast<FaceControl*>(fc));    // Calls doAction on this
}   // end ensureProcessed


bool ActionMapSurfaceData::testReady( FaceControl* fc) { return _cmaps.count(fc->data()) == 0;}


bool ActionMapSurfaceData::doAction( FaceControlSet& rset)
{
    for ( const FaceModel* fm : rset.models())
    {
        assert(_cmaps.count(fm) == 0);
        _cmaps[fm] = new SurfaceData( fm->info());
    }   // end for
    return true;
}   // end doAction


void ActionMapSurfaceData::respondToChange( FaceControl* fc)
{
    if ( _cmaps.count(fc->data()) > 0)
    {
        burn(fc);
        process(fc);    // Re-map the curvature
    }   // end if
    FaceAction::respondToChange(fc);    // Re-test
}   // end respondToChange


void ActionMapSurfaceData::burn( const FaceControl* fc)
{
    const FaceModel* fm = fc->data();
    if ( _cmaps.count(fm) > 0)
    {
        delete _cmaps.at(fm);
        _cmaps.erase(fm);
    }   // end if
}   // end burn
