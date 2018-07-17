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

#include <FaceTools.h>
#include <FaceShapeLandmarks2DDetector.h>   // namespace FaceTools::Landmarks
#include <Transformer.h>    // RFeatures
#include <algorithm>
using namespace RFeatures;


bool FaceTools::hasReqLandmarks( const LandmarkSet::Ptr lset)
{
    using namespace FaceTools::Landmarks;
    return lset->has( L_EYE_CENTRE) && lset->has(R_EYE_CENTRE) && lset->has(NASAL_TIP);
}   // end hasReqLandmarks


cv::Vec3f FaceTools::calcFaceCentre( const cv::Vec3f& upv, const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& nt)
{
    const cv::Vec3f midEye = (v0 + v1) * 0.5;
    const cv::Vec3f dvec = nt - midEye;
    cv::Vec3f downVec;
    cv::normalize( -upv, downVec);
    return midEye + (downVec.dot(dvec) * downVec);
}   // end calcFaceCentre


cv::Vec3f FaceTools::calcFaceCentre( const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& nt)
{
    return (1.0f/3) * (v0 + v1 + nt);
}   // end calcFaceCentre


double FaceTools::calcFaceCropRadius( const cv::Vec3f& fcentre, const cv::Vec3f& v0, const cv::Vec3f& v1, double G)
{
    return G * (cv::norm( fcentre - v0) + cv::norm( fcentre - v1))/2;
}   // end calcFaceCropRadius


ObjModel::Ptr FaceTools::createFromVertices( const cv::Mat_<cv::Vec3f>& vrow)
{
    ObjModel::Ptr omod = ObjModel::create();
    const int npoints = vrow.cols;
    const cv::Vec3f* vptr = vrow.ptr<cv::Vec3f>(0);
    for ( int i = 0; i < npoints; ++i)
        omod->addVertex( vptr[i]);
    return omod;
}   // end createFromVertices


ObjModel::Ptr FaceTools::createFromSubset( const ObjModel* smod, const IntSet& vidxs)
{
    assert(smod);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    std::for_each( std::begin(vidxs), std::end(vidxs), [&](int vidx){ omod->addVertex( smod->vtx(vidx));});
    return omod;
}   // end createFromSubset


ObjModel::Ptr FaceTools::createFromTransformedSubset( const ObjModel* smod, const IntSet& vidxs, const cv::Matx44d& T,
                                                      std::unordered_map<int,int>* newVidxsToSource)
{
    assert(smod);
    const RFeatures::Transformer transformer(T);
    ObjModel::Ptr omod = ObjModel::create( smod->getSpatialPrecision());
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = smod->vtx(vidx);
        transformer.transform( v);
        const int nvidx = omod->addVertex(v);
        if ( newVidxsToSource)
            (*newVidxsToSource)[nvidx] = vidx;
    }   // end for
    return omod;
}   // end createFromTransformedSubset


// Flatten m to XY plane and return it, also setting fmap to be the
// vertex ID mapping from the returned flattened object to the original object m.
ObjModel::Ptr FaceTools::makeFlattened( const ObjModel* m, std::unordered_map<int,int>* fmap)
{
    assert(m);
    if ( fmap)
        fmap->clear();
    const IntSet& vidxs = m->getVertexIds();
    int nvidx;
    ObjModel::Ptr fmod = ObjModel::create( m->getSpatialPrecision());
    for ( int vidx : vidxs)
    {
        cv::Vec3f v = m->vtx(vidx);
        v[2] = 0;
        nvidx = fmod->addVertex(v);
        if ( fmap)
            (*fmap)[nvidx] = vidx;
    }   // end for
    // fmod is now a flattened version
    return fmod;
}   // end makeFlattened
