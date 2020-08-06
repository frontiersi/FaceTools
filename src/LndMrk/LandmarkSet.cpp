/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <LndMrk/LandmarkSet.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <r3d/SurfacePointFinder.h>
#include <cassert>
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::Landmark;
using FaceTools::Landmark::LmkList;
using FaceTools::Landmark::SpecificLandmark;
using FaceTools::FaceLateral;
using FaceTools::Vec3f;
using FaceTools::Mat4f;
using FaceTools::MatX3f;
using LDMRK_PAIR = std::pair<int, Vec3f>;
using LMAN = FaceTools::Landmark::LandmarksManager;


namespace  {
void addLandmarks( const std::unordered_map<int, Vec3f>& vs, FaceTools::FaceLateral lat, LandmarkSet &mlmks)
{
    for ( const auto& p : vs)
    {
        const int lmid = p.first;
        Vec3f v = Vec3f::Zero();
        if ( mlmks.has(lmid, lat))
            v = mlmks.pos(lmid, lat);
        v += p.second;
        mlmks.set( lmid, v, lat);
    }   // end for
}   // end addLandmarks


void setAverage( std::unordered_map<int, Vec3f>& lat, int n)
{
    for ( auto& p : lat)
        lat[p.first] = p.second / n;
}   // end setAverage

}   // end namespace


LandmarkSet::LandmarkSet( const std::unordered_set<const LandmarkSet*>& lms)
{
    int n = 0;
    for ( const LandmarkSet* lmks : lms)
    {
        if ( lmks->empty())
            continue;

        n++;
        const std::unordered_map<int, Vec3f>& llat = lmks->lateral(FACE_LATERAL_LEFT);
        const std::unordered_map<int, Vec3f>& mlat = lmks->lateral(FACE_LATERAL_MEDIAL);
        const std::unordered_map<int, Vec3f>& rlat = lmks->lateral(FACE_LATERAL_RIGHT);

        addLandmarks( llat, FACE_LATERAL_LEFT, *this);
        addLandmarks( mlat, FACE_LATERAL_MEDIAL, *this);
        addLandmarks( rlat, FACE_LATERAL_RIGHT, *this);
    }   // end for

    if ( n > 0)
    {
        setAverage( _lmksL, n);
        setAverage( _lmksM, n);
        setAverage( _lmksR, n);
    }   // end if

    //_clearAlignment();
}   // end ctor

/*
void LandmarkSet::_clearAlignment() const
{
    _algn.setZero();
    _ialgn.setZero();
}   // end _clearAlignment
*/


bool LandmarkSet::has( int id, FaceLateral lat) const
{
    if ( !has(id))
        return false;

    bool hasLmk = false;
    switch (lat)
    {
        case FACE_LATERAL_LEFT:
            hasLmk = _lmksL.count(id) > 0;
            break;
        case FACE_LATERAL_MEDIAL:
            hasLmk = _lmksM.count(id) > 0;
            break;
        case FACE_LATERAL_RIGHT:
            hasLmk = _lmksR.count(id) > 0;
            break;
    }   // end switch
    return hasLmk;
}   // end has


bool LandmarkSet::has( const SpecificLandmark& p) const { return has(p.id, p.lat);}


const LandmarkSet::LDMRKS& LandmarkSet::lateral( FaceLateral lat) const
{
    const LDMRKS* lmks = nullptr;
    switch (lat)
    {
        case FACE_LATERAL_LEFT:
            lmks = &_lmksL;
            break;
        case FACE_LATERAL_MEDIAL:
            lmks = &_lmksM;
            break;
        case FACE_LATERAL_RIGHT:
            lmks = &_lmksR;
            break;
    }   // end switch
    return *lmks;
}   // end lateral


LandmarkSet::LDMRKS& LandmarkSet::_lateral( FaceLateral lat)
{
    const LandmarkSet* me = this;
    return const_cast<LDMRKS&>( me->lateral(lat));
}   // end _lateral


bool LandmarkSet::set( int id, const Vec3f& v, FaceLateral lat)
{
    Landmark* lmk = LMAN::landmark(id);
    if ( !lmk)
        return false;

    if ( !lmk->isBilateral())   // Ignore specified lateral if landmark is not bilateral
        lat = FACE_LATERAL_MEDIAL;

    _lateral(lat)[id] = v;
    _ids.insert(id);

    /*
    if ( LMAN::usedForAlignment(id))
        _clearAlignment();
    */
    return true;
}   // end set


bool LandmarkSet::set( const QString& lmcode, const Vec3f& v, FaceLateral lat)
{
    Landmark* lmk = LMAN::landmark(lmcode);
    if ( !lmk)
        return false;
    return set( lmk->id(), v, lat);
}   // end set


const Vec3f& LandmarkSet::pos( int id, FaceLateral lat) const
{
    static const Vec3f ZERO_VEC = Vec3f::Zero();    // Otherwise returning reference to temporary

    if ( LMAN::landmark(id)->isBilateral() && lat == FACE_LATERAL_MEDIAL)
    {
        std::cerr << "[ERROR] FaceTools::Landmark::LandmarkSet::pos: Requested landmark is bilateral but requested medial!" << std::endl;
        assert(false);
        return ZERO_VEC;
    }   // end if

    assert(has(id, lat));
    if (!has(id, lat))
        return ZERO_VEC;

    return lateral(lat).at(id);
}   // end pos


const Vec3f& LandmarkSet::pos( const QString& lmcode, FaceLateral lat) const
{
    return pos( LMAN::landmark(lmcode)->id(), lat);
}   // end pos


const Vec3f& LandmarkSet::pos( const SpecificLandmark& sl) const { return pos( sl.id, sl.lat);}


Vec3f LandmarkSet::toPoint( const LmkList &ll, const Mat4f& T, const Mat4f& iT) const
{
    Vec3f v = Vec3f::Zero();
    for ( const SpecificLandmark &slmk : ll)
        v += slmk.prop.cwiseProduct( r3d::transform( iT, pos(slmk)));
    return r3d::transform( T, v);
}   // end toPoint


r3d::Mesh::Ptr LandmarkSet::toMesh() const
{
    std::vector<int> lmids( ids().begin(), ids().end());
    std::sort( lmids.begin(), lmids.end());

    r3d::Mesh::Ptr mesh = r3d::Mesh::create();
    for ( int id : lmids)
    {
        if ( LMAN::landmark(id)->isBilateral())
        {
            mesh->addVertex( pos( id, FACE_LATERAL_LEFT));
            mesh->addVertex( pos( id, FACE_LATERAL_RIGHT));
        }   // end if
        else
            mesh->addVertex( pos( id, FACE_LATERAL_MEDIAL));
    }   // end for

    return mesh;
}   // end toMesh


Vec3f LandmarkSet::eyeVec() const
{
    Vec3f v = Vec3f::Zero();
    if ( has( LMAN::codeId(P)))
    {
        const int id = LMAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = pos( id, FACE_LATERAL_RIGHT) - pos( id, FACE_LATERAL_LEFT);
    }   // end if
    return v;
}   // end eyeVec


Vec3f LandmarkSet::midEyePos() const
{
    Vec3f v = Vec3f::Zero();
    if ( has( LMAN::codeId(P)))
    {
        const int id = LMAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = 0.5f*(pos( id, FACE_LATERAL_RIGHT) + pos( id, FACE_LATERAL_LEFT));
    }   // end if
    return v;
}   // end midEyePos

namespace {

Vec3f getMeanOfSet( const IntSet &ms, const std::unordered_map<int, Vec3f> &lmks)
{
    int cnt = 0;
    Vec3f p = Vec3f::Zero();
    for ( int lid : ms)
    {
        if ( lmks.count(lid) > 0)
        {
            p += lmks.at(lid);
            cnt++;
        }   // end if
    }   // end for
    if ( cnt > 0)
        p /= cnt;
    return p;
}   // end getMeanOfSet

}   // end namespace


Vec3f LandmarkSet::medialMean() const { return getMeanOfSet( LMAN::medialAlignmentSet(), _lmksM);}

/*
Vec3f LandmarkSet::_quarter0() const { return getMeanOfSet( LMAN::topAlignmentSet(), _lmksR);}
Vec3f LandmarkSet::_quarter1() const { return getMeanOfSet( LMAN::topAlignmentSet(), _lmksL);}
Vec3f LandmarkSet::_quarter2() const { return getMeanOfSet( LMAN::bottomAlignmentSet(), _lmksL);}
Vec3f LandmarkSet::_quarter3() const { return getMeanOfSet( LMAN::bottomAlignmentSet(), _lmksR);}


Mat4f LandmarkSet::alignment() const
{
    if ( empty())
        return Mat4f::Identity();

    if ( _algn.isZero())
    {
        const Vec3f mmean = medialMean();

        const Vec3f q0 = _quarter0() - mmean;
        const Vec3f q1 = _quarter1() - mmean;
        const Vec3f q2 = _quarter2() - mmean;
        const Vec3f q3 = _quarter3() - mmean;

        Vec3f zvec01 = q0.cross(q1);
        Vec3f zvec12 = q1.cross(q2);
        Vec3f zvec23 = q2.cross(q3);
        Vec3f zvec30 = q3.cross(q0);
        zvec01.normalize();
        zvec12.normalize();
        zvec23.normalize();
        zvec30.normalize();

        Vec3f zvec = zvec01 + zvec12 + zvec23 + zvec30;
        Vec3f yvec = (q0 + q1) - (q2 + q3);
        Vec3f xvec = yvec.cross(zvec);
        zvec = xvec.cross(yvec);

        xvec.normalize();
        yvec.normalize();
        zvec.normalize();
        // Only now are these vectors orthonormal.

        _algn = Mat4f::Identity();
        _algn.block<3,1>(0,0) = xvec;
        _algn.block<3,1>(0,1) = yvec;
        _algn.block<3,1>(0,2) = zvec;

        // Set the mean further back toward the ear
        //const float d = (q0 - q1).norm() + (q1 - q2).norm() + (q2 - q3).norm() + (q3 - q0).norm();
        _algn.block<3,1>(0,3) = mmean;// - 0.3f * d * zvec;
        _ialgn.setZero();
    }   // end if

    assert( !_algn.isZero());
    return _algn;
}   // end alignment


Mat4f LandmarkSet::inverseAlignment() const
{
    if ( empty())
        return Mat4f::Identity();

    if ( _ialgn.isZero())
    {
        if ( _algn.isZero())    // Ensure the alignment matrix is calculated
            alignment();
        _ialgn = _algn.inverse();
    }   // end if

    assert( !_ialgn.isZero());
    return _ialgn;
}   // end inverseAlignment
*/


namespace {

void testSetExtreme( Vec3f &minc, Vec3f &maxc, const Vec3f &v)
{
    minc[0] = std::min( minc[0], v[0]);
    maxc[0] = std::max( maxc[0], v[0]);
    minc[1] = std::min( minc[1], v[1]);
    maxc[1] = std::max( maxc[1], v[1]);
    minc[2] = std::min( minc[2], v[2]);
    maxc[2] = std::max( maxc[2], v[2]);
}   // end testSetExtreme

}   // end namespace


r3d::Bounds::Ptr LandmarkSet::makeBounds( const Mat4f &T, const Mat4f &iT) const
{
    Vec3f minc( FLT_MAX, FLT_MAX, FLT_MAX);
    Vec3f maxc = -minc;

    for ( const auto& p : _lmksL)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));
    for ( const auto& p : _lmksM)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));
    for ( const auto& p : _lmksR)
        testSetExtreme( minc, maxc, r3d::transform( iT, p.second));

    const Vec3f cen = r3d::transform( iT, medialMean());

    static const float X_FACTOR = 1.0f;
    static const float Yt_FACTOR = 1.0f;
    static const float Yb_FACTOR = 1.0f;
    static const float Z_FACTOR = 1.0f;
    minc[0] = cen[0] - X_FACTOR * fabsf(minc[0] - cen[0]);
    maxc[0] = cen[0] + X_FACTOR * fabsf(maxc[0] - cen[0]);
    minc[1] = cen[1] - Yb_FACTOR * fabsf(minc[1] - cen[1]);
    maxc[1] = cen[1] + Yt_FACTOR * fabsf(maxc[1] - cen[1]);
    minc[2] = cen[2] - Z_FACTOR * fabsf(minc[2] - cen[2]);
    
    r3d::Bounds::Ptr bnds = r3d::Bounds::create( minc, maxc);
    bnds->setTransformMatrix( T);
    return bnds;
}   // end makeBounds


float LandmarkSet::sqRadius() const
{
    float sqDist = 0.0f;
    const Vec3f mean = medialMean();
    for ( const auto& p : _lmksL)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    for ( const auto& p : _lmksM)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    for ( const auto& p : _lmksR)
        sqDist = std::max( (p.second - mean).squaredNorm(), sqDist);
    return sqDist;
}   // end sqRadius


void LandmarkSet::swapLaterals()
{
    const auto tmp = _lmksL;
    _lmksL = _lmksR;
    _lmksR = tmp;
    //_clearAlignment();
}   // end swapLaterals


void LandmarkSet::moveToSurface( const FaceTools::FM* fm)
{
    for ( const auto& p : _lmksL)
        _lmksL.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
    for ( const auto& p : _lmksM)
        _lmksM.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
    for ( const auto& p : _lmksR)
        _lmksR.at(p.first) = FaceTools::toSurface( fm->kdtree(), p.second);
    //_clearAlignment();
}   // end moveToSurface


void LandmarkSet::transform( const Mat4f& t)
{
    assert( !t.isZero());
    for ( auto& p : _lmksL)
        p.second = r3d::transform( t, p.second);
    for ( auto& p : _lmksM)
        p.second = r3d::transform( t, p.second);
    for ( auto& p : _lmksR)
        p.second = r3d::transform( t, p.second);
    //_clearAlignment();
}   // end transform


// Write out the landmarks to record.
void LandmarkSet::write( PTree& lnodes) const
{
    PTree& llat = lnodes.put("LeftLateral", "");
    for ( const auto& p : _lmksL)
        r3d::putNamedVertex( llat, LMAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& mlat = lnodes.put("Medial", "");
    for ( const auto& p : _lmksM)
        r3d::putNamedVertex( mlat, LMAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& rlat = lnodes.put("RightLateral", "");
    for ( const auto& p : _lmksR)
        r3d::putNamedVertex( rlat, LMAN::landmark(p.first)->code().toStdString(), p.second);
}   // end write


bool LandmarkSet::_readLateral( const PTree& lats, FaceLateral lat)
{
    std::string tag = "Medial";
    if ( lat == FACE_LATERAL_LEFT)
        tag = "LeftLateral";
    else if ( lat == FACE_LATERAL_RIGHT)
        tag = "RightLateral";

    if ( lats.count(tag) == 0)
    {
        std::cerr << "[ERROR] FaceTools::Landmark::LandmarkSet::_readLateral: Didn't find landmark lateral tag!" << std::endl;
        return false;
    }   // end if

    for ( const PTree::value_type& lval : lats.get_child( tag))
    {
        const QString lmcode = lval.first.c_str();
        Landmark* lmk = LMAN::landmark( lmcode);
        if ( !lmk)
        {
#ifndef NDEBUG
            std::cerr << "[WARNING] FaceTools::Landmark::LandmarkSet::_readLateral: Unable to read landmark '"
                      << lmcode.toStdString() << "'; not present in LMAN!" << std::endl;
#endif
            return false;
        }   // end if
        set( lmk->id(), r3d::getVertex(lval.second), lat);
    }   // end for
    return true;
}   // end _readLateral


// Read in the landmarks from record.
bool LandmarkSet::read( const PTree& lnodes)
{
    //_clearAlignment();
    if ( !_readLateral( lnodes, FACE_LATERAL_LEFT))
        return false;
    if ( !_readLateral( lnodes, FACE_LATERAL_MEDIAL))
        return false;
    if ( !_readLateral( lnodes, FACE_LATERAL_RIGHT))
        return false;
    return true;
}   // end read


namespace {
void checkCloser( float &minSqDist, Vec3f &nv, const Vec3f &vm, const Vec3f &v, float snapSqDist)
{
    const float sqdist = (vm - v).squaredNorm();
    if (sqdist < snapSqDist && sqdist < minSqDist)
    {
        minSqDist = sqdist;
        nv = vm;
    }   // end if
}   // end checkCloser
}   // end namespace


Vec3f LandmarkSet::snapToVisible( const Vec3f &v, float snapSqDist) const
{
    Vec3f nv = v;
    float minSqDist = FLT_MAX;
    for ( int lmid : _ids)
    {
        if ( LMAN::landmark(lmid)->isVisible())
        {
            if ( LMAN::isBilateral(lmid))
            {
                checkCloser( minSqDist, nv, pos(lmid, FACE_LATERAL_LEFT), v, snapSqDist);
                checkCloser( minSqDist, nv, pos(lmid, FACE_LATERAL_RIGHT), v, snapSqDist);
            }   // end if
            else
                checkCloser( minSqDist, nv, pos(lmid, FACE_LATERAL_MEDIAL), v, snapSqDist);
        }   // end if
    }   // end for
    return nv;
}   // end snapToVisible
