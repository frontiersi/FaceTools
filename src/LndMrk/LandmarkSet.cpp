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

#include <LandmarkSet.h>
#include <FaceTools.h>
#include <Transformer.h>  // RFeatures
#include <ObjModelSurfacePointFinder.h>
#include <Orientation.h>
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::Landmark;
using FaceTools::FaceLateral;
using LDMRK_PAIR = std::pair<int, cv::Vec3f>;
#include <algorithm>
#include <cassert>

LandmarkSet::Ptr LandmarkSet::create() { return Ptr( new LandmarkSet, [](LandmarkSet* d){ delete d;});}

LandmarkSet::LandmarkSet() {}


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


// private
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

    lateral(lat)[id] = v;
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


const cv::Vec3f* LandmarkSet::pos( int id, FaceLateral lat) const
{
    if ( LDMKS_MAN::landmark(id)->isBilateral() && lat == FACE_LATERAL_MEDIAL)
    {
        std::cerr << "[WARNING] FaceTools::Landmark::LandmarkSet::pos: Requested landmark is bilateral but requested medial!" << std::endl;
        return nullptr;
    }   // end if

    if (!has(id, lat))
        return nullptr;
    return &lateral(lat).at(id);
}   // end pos


const cv::Vec3f* LandmarkSet::pos( const QString& lmcode, FaceLateral lat) const
{
    const int id = LDMKS_MAN::landmark(lmcode)->id(); // Get the id of the pupil landmark
    return pos( id, lat);
}   // end pos


cv::Vec3f LandmarkSet::eyeVec() const
{
    cv::Vec3f v(0,0,0);
    if ( hasCode(P))
    {
        const int id = LDMKS_MAN::landmark(P)->id(); // Get the id of the pupil landmark
        v = *pos( id, FACE_LATERAL_RIGHT) - *pos( id, FACE_LATERAL_LEFT);
    }   // end if
    return v;
}   // end eyeVec


bool LandmarkSet::translate( int id, FaceLateral lat, const cv::Vec3f& t)
{
    if (!has(id,lat))
        return false;
    lateral(lat).at(id) += t;
    return true;
}   // end translate


void LandmarkSet::translate( const cv::Vec3f& t)
{
    for ( auto& p : _lmksL)
        translate( p.first, FACE_LATERAL_LEFT, t);
    for ( auto& p : _lmksM)
        translate( p.first, FACE_LATERAL_MEDIAL, t);
    for ( auto& p : _lmksR)
        translate( p.first, FACE_LATERAL_RIGHT, t);
}   // end translate


void LandmarkSet::transform( const cv::Matx44d& T)
{
    const RFeatures::Transformer mover(T);
    for ( auto& p : _lmksL)
        mover.transform( _lmksL.at(p.first));
    for ( auto& p : _lmksM)
        mover.transform( _lmksM.at(p.first));
    for ( auto& p : _lmksR)
        mover.transform( _lmksR.at(p.first));
}   // end transform


void LandmarkSet::moveToSurface( const RFeatures::ObjModelKDTree* kdt)
{
    for ( const auto& p : _lmksL)
        _lmksL.at(p.first) = FaceTools::toSurface( kdt, p.second);
    for ( const auto& p : _lmksM)
        _lmksM.at(p.first) = FaceTools::toSurface( kdt, p.second);
    for ( const auto& p : _lmksR)
        _lmksR.at(p.first) = FaceTools::toSurface( kdt, p.second);
}   // end moveToSurface


// Write out the landmarks to record.
void LandmarkSet::write( PTree& lnodes) const
{
    PTree& llat = lnodes.put("LeftLateral", "");
    for ( const auto& p : _lmksL)
        RFeatures::putNamedVertex( llat, LDMKS_MAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& mlat = lnodes.put("Medial", "");
    for ( const auto& p : _lmksM)
        RFeatures::putNamedVertex( mlat, LDMKS_MAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& rlat = lnodes.put("RightLateral", "");
    for ( const auto& p : _lmksR)
        RFeatures::putNamedVertex( rlat, LDMKS_MAN::landmark(p.first)->code().toStdString(), p.second);
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
