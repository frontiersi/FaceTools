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

#include <LandmarkSet.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <ObjModelSurfacePointFinder.h>
#include <Orientation.h>
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::Landmark;
using FaceTools::Landmark::SpecificLandmark;
using FaceTools::FaceLateral;
using FaceTools::FM;
using LDMRK_PAIR = std::pair<int, cv::Vec3f>;
#include <algorithm>
#include <cassert>


namespace  {
void addLandmarks( const std::unordered_map<int, cv::Vec3f>& vs, FaceTools::FaceLateral lat, LandmarkSet::Ptr mlmks)
{
    for ( const auto& p : vs)
    {
        const int lmid = p.first;
        cv::Vec3f v(0,0,0);
        if ( mlmks->has(lmid, lat))
            v = mlmks->upos(lmid, lat);
        v += p.second;
        mlmks->set( lmid, v, lat);
    }   // end for
}   // end addLandmarks


void setAverage( std::unordered_map<int, cv::Vec3f>& lat, int n)
{
    const double sf = 1.0/n;
    for ( auto& p : lat)
    {
        const cv::Vec3f v = p.second * sf;
        lat[p.first] = v;
    }   // end for
}   // end setAverage

}   // end namespace


LandmarkSet::Ptr LandmarkSet::createMean( const std::unordered_set<const LandmarkSet*>& lms)
{
    LandmarkSet::Ptr mlmks = LandmarkSet::create();
    cv::Matx44d tmat = cv::Matx44d::zeros();

    int n = 0;
    for ( const LandmarkSet* lmks : lms)
    {
        if ( lmks->empty())
            continue;

        n++;
        tmat += lmks->transformMatrix();
        const std::unordered_map<int, cv::Vec3f>& llat = lmks->lateral(FACE_LATERAL_LEFT);
        const std::unordered_map<int, cv::Vec3f>& mlat = lmks->lateral(FACE_LATERAL_MEDIAL);
        const std::unordered_map<int, cv::Vec3f>& rlat = lmks->lateral(FACE_LATERAL_RIGHT);

        addLandmarks( llat, FACE_LATERAL_LEFT, mlmks);
        addLandmarks( mlat, FACE_LATERAL_MEDIAL, mlmks);
        addLandmarks( rlat, FACE_LATERAL_RIGHT, mlmks);
    }   // end for

    if ( n > 0)
    {
        setAverage( mlmks->_lmksL, n);
        setAverage( mlmks->_lmksM, n);
        setAverage( mlmks->_lmksR, n);
        // Set the mean transform matrix
        tmat *= 1.0/n;
        mlmks->addTransformMatrix(tmat);
    }   // end if

    return mlmks;
}   // end createMean


LandmarkSet::Ptr LandmarkSet::create() { return Ptr( new LandmarkSet, [](LandmarkSet* d){ delete d;});}


LandmarkSet::Ptr LandmarkSet::deepCopy() const
{
    return Ptr( new LandmarkSet(*this), [](LandmarkSet* d){ delete d;});
}   // end deepCopy


LandmarkSet::LandmarkSet() : _tmat(cv::Matx44d::eye()), _imat(cv::Matx44d::eye()) {}


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


// private
LandmarkSet::LDMRKS& LandmarkSet::lateral( FaceLateral lat)
{
    const LandmarkSet* me = this;
    return const_cast<LDMRKS&>( me->lateral(lat));
}   // end lateral


bool LandmarkSet::set( int id, const cv::Vec3f& v, FaceLateral lat)
{
    Landmark* lmk = LDMKS_MAN::landmark(id);
    if ( !lmk)
        return false;

    // Laterality argument ignored if landmark not bilateral.
    if ( !lmk->isBilateral())
        lat = FACE_LATERAL_MEDIAL;

    lateral(lat)[id] = RFeatures::transform( _imat, v);
    _names.insert(lmk->name());
    _codes.insert(lmk->code());
    _ids.insert(id);
    return true;
}   // end set


bool LandmarkSet::set( const QString& lmcode, const cv::Vec3f& v, FaceLateral lat)
{
    Landmark* lmk = LDMKS_MAN::landmark(lmcode);
    if ( !lmk)
        return false;
    return set( lmk->id(), v, lat);
}   // end set


bool LandmarkSet::erase( int id)
{
    Landmark* lmk = LDMKS_MAN::landmark(id);
    if ( !lmk)
        return false;

    _ids.erase(id);
    _lmksL.erase(id);
    _lmksM.erase(id);
    _lmksR.erase(id);
    _names.erase(lmk->name());
    _codes.erase(lmk->code());
    return true;
}   // end erase


void LandmarkSet::clear()
{
    IntSet ids = _ids;  // Copy out
    for ( int id : ids)
        erase(id);
}   // end clear


cv::Vec3f LandmarkSet::pos( int id, FaceLateral lat) const
{
    return RFeatures::transform( _tmat, upos( id, lat));
}   // end pos


const cv::Vec3f& LandmarkSet::upos( int id, FaceLateral lat) const
{
    static const cv::Vec3f ZEROV(0,0,0);
    if ( LDMKS_MAN::landmark(id)->isBilateral() && lat == FACE_LATERAL_MEDIAL)
    {
        std::cerr << "[ERROR] FaceTools::Landmark::LandmarkSet::pos: Requested landmark is bilateral but requested medial!" << std::endl;
        assert(false);
        return ZEROV;
    }   // end if

    assert(has(id, lat));
    if (!has(id, lat))
        return ZEROV;

    return lateral(lat).at(id);
}   // end upos


cv::Vec3f LandmarkSet::pos( const QString& lmcode, FaceLateral lat) const
{
    return pos( LDMKS_MAN::landmark(lmcode)->id(), lat);
}   // end pos


const cv::Vec3f& LandmarkSet::upos( const QString& lmcode, FaceLateral lat) const
{
    return upos( LDMKS_MAN::landmark(lmcode)->id(), lat);
}   // end upos


cv::Vec3f LandmarkSet::pos( const SpecificLandmark& sl) const { return pos( sl.id, sl.lat);}


cv::Vec3f LandmarkSet::posSomeMedial() const
{
    assert( !_lmksM.empty());
    if ( _lmksM.empty())
        return cv::Vec3f(0,0,0);
    return _lmksM.begin()->second;
}   // end posSomeMedial


cv::Vec3f LandmarkSet::rightVec() const
{
    if ( empty())
        return cv::Vec3f(1,0,0);

    cv::Vec3f rv(0,0,0);
    for ( const auto& p : _lmksL)
    {
        int lmid = p.first;
        cv::Vec3f lpos = pos( lmid, FACE_LATERAL_LEFT);
        cv::Vec3f rpos = pos( lmid, FACE_LATERAL_RIGHT);
        // Landmarks further apart laterally more greatly influence the determination of this vector.
        rv += rpos - lpos;
    }   // end for

    cv::Vec3f urv;
    cv::normalize( rv, urv);
    return urv;
}   // end rightVec


cv::Vec3f LandmarkSet::upVec() const
{
    if ( empty())
        return cv::Vec3f(0,1,0);

    cv::Vec3f uvec;
    cv::normalize( superiorMean() - inferiorMean(), uvec);
    return uvec;
}   // end upVec


cv::Vec3f LandmarkSet::normVec() const
{
    if ( empty())
        return cv::Vec3f(0,0,1);
    cv::Vec3f nvec;
    cv::normalize( rightVec().cross(upVec()), nvec);
    return nvec;
}   // end normVec


RFeatures::Orientation LandmarkSet::orientation() const
{
    cv::Vec3f uvec = upVec();
    cv::Vec3f rvec = rightVec();
    cv::Vec3f nvec;
    cv::normalize( rvec.cross(uvec), nvec);
    return RFeatures::Orientation( nvec, uvec);
}   // end orientation


cv::Vec3f LandmarkSet::eyeVec() const
{
    cv::Vec3f v(0,0,0);
    if ( hasCode(P))
    {
        const int id = LDMKS_MAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = pos( id, FACE_LATERAL_RIGHT) - pos( id, FACE_LATERAL_LEFT);
    }   // end if
    return v;
}   // end eyeVec


cv::Vec3f LandmarkSet::superiorMean() const
{
    cv::Vec3f m(0,0,0);
    if ( hasCode(MSO))
        m += pos( MSO, FACE_LATERAL_LEFT) + pos( MSO, FACE_LATERAL_RIGHT);
    if ( hasCode(EX))
        m += pos(  EX, FACE_LATERAL_LEFT) + pos(  EX, FACE_LATERAL_RIGHT);
    if ( hasCode(EN))
        m += pos(  EN, FACE_LATERAL_LEFT) + pos(  EN, FACE_LATERAL_RIGHT);
    if ( hasCode(G))
        m += pos(   G);
    if ( hasCode(N))
        m += pos(   N);
    if ( hasCode(SE))
        m += pos(  SE);
    if ( hasCode(MND))
        m += pos( MND);
    return m * 1.0f/10;
}   // end superiorMean


cv::Vec3f LandmarkSet::inferiorMean() const
{
    cv::Vec3f m(0,0,0);
    if ( hasCode(AC))
        m += pos(  AC, FACE_LATERAL_LEFT) + pos(  AC, FACE_LATERAL_RIGHT);
    if ( hasCode(CPH))
        m += pos( CPH, FACE_LATERAL_LEFT) + pos( CPH, FACE_LATERAL_RIGHT);
    if ( hasCode(CH))
        m += pos(  CH, FACE_LATERAL_LEFT) + pos(  CH, FACE_LATERAL_RIGHT);
    if ( hasCode(LS))
        m += pos(  LS);
    if ( hasCode(LI))
        m += pos(  LI);
    if ( hasCode(STS))
        m += (pos( STS) + pos( STI)) * 0.5f;
    if ( hasCode(SL))
        m += pos(  SL);
    return m * 1.0f/10;
}   // end inferiorMean


cv::Vec3f LandmarkSet::fullMean() const
{
    return 0.5f*(superiorMean() + inferiorMean());
}   // end fullMean


void LandmarkSet::swapLaterals()
{
    const auto tmp = _lmksL;
    _lmksL = _lmksR;
    _lmksR = tmp;
}   // end swapLaterals


void LandmarkSet::moveToSurface( const FM* fm)
{
    for ( const auto& p : _lmksL)
        _lmksL.at(p.first) = RFeatures::transform( _imat, FaceTools::toSurface( fm, RFeatures::transform( _tmat, p.second)));
    for ( const auto& p : _lmksM)
        _lmksM.at(p.first) = RFeatures::transform( _imat, FaceTools::toSurface( fm, RFeatures::transform( _tmat, p.second)));
    for ( const auto& p : _lmksR)
        _lmksR.at(p.first) = RFeatures::transform( _imat, FaceTools::toSurface( fm, RFeatures::transform( _tmat, p.second)));
}   // end moveToSurface


void LandmarkSet::addTransformMatrix(const cv::Matx44d &tmat)
{
    _tmat = tmat * _tmat;
    _imat = _tmat.inv();
}   // end addTransformMatrix


void LandmarkSet::fixTransformMatrix()
{
    for ( auto& p : _lmksL)
        RFeatures::transform( _tmat, p.second);
    for ( auto& p : _lmksM)
        RFeatures::transform( _tmat, p.second);
    for ( auto& p : _lmksR)
        RFeatures::transform( _tmat, p.second);
    _tmat = _imat = cv::Matx44d::eye();
}   // end fixTransformMatrix


// Write out the landmarks to record.
void LandmarkSet::write( PTree& lnodes) const
{
    PTree& llat = lnodes.put("LeftLateral", "");
    for ( const auto& p : _lmksL)
        RFeatures::putNamedVertex( llat, LDMKS_MAN::landmark(p.first)->code().toStdString(), RFeatures::transform( _tmat, p.second));
    PTree& mlat = lnodes.put("Medial", "");
    for ( const auto& p : _lmksM)
        RFeatures::putNamedVertex( mlat, LDMKS_MAN::landmark(p.first)->code().toStdString(), RFeatures::transform( _tmat, p.second));
    PTree& rlat = lnodes.put("RightLateral", "");
    for ( const auto& p : _lmksR)
        RFeatures::putNamedVertex( rlat, LDMKS_MAN::landmark(p.first)->code().toStdString(), RFeatures::transform( _tmat, p.second));
}   // end write


// private
bool LandmarkSet::readLateral( const PTree& lats, FaceLateral lat)
{
    std::string tag = "Medial";
    if ( lat == FACE_LATERAL_LEFT)
        tag = "LeftLateral";
    else if ( lat == FACE_LATERAL_RIGHT)
        tag = "RightLateral";

    if ( lats.count(tag) == 0)
    {
        std::cerr << "[ERROR FaceTools::Landmark::LandmarkSet::readLateral: Didn't find landmark lateral tag!" << std::endl;
        return false;
    }   // end if

    for ( const PTree::value_type& lval : lats.get_child( tag))
    {
        const QString lmcode = lval.first.c_str();
        Landmark* lmk = LDMKS_MAN::landmark( lmcode);
        if ( !lmk)
        {
            std::cerr << "[WARNING] FaceTools::Landmark::LandmarkSet::readLateral: Unable to read landmark '"
                      << lmcode.toStdString() << "'; not present in LandmarksManager!" << std::endl;
            return false;
        }   // end if
        set( lmk->id(), RFeatures::getVertex(lval.second), lat);
    }   // end for
    return true;
}   // end readLateral


// Read in the landmarks from record.
bool LandmarkSet::read( const PTree& lnodes)
{
    if ( !readLateral( lnodes, FACE_LATERAL_LEFT))
        return false;
    if ( !readLateral( lnodes, FACE_LATERAL_MEDIAL))
        return false;
    if ( !readLateral( lnodes, FACE_LATERAL_RIGHT))
        return false;
    return true;
}   // end read
