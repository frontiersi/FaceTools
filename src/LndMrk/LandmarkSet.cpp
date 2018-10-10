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
#include <Transformer.h>  // RFeatures
#include <ObjModelSurfacePointFinder.h>
#include <Orientation.h>
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Landmark::Landmark;
using LDMRK_PAIR = std::pair<int, cv::Vec3f>;
#include <algorithm>
#include <cassert>

LandmarkSet::Ptr LandmarkSet::create() { return Ptr( new LandmarkSet, [](LandmarkSet* d){ delete d;});}

LandmarkSet::LandmarkSet() : _lp(&_lmksL), _rp(&_lmksR), _view(nullptr) {}


void LandmarkSet::registerViewer( FaceTools::Vis::LandmarkSetView* v)
{
    _view = v;
    assert(_view);
    for ( const auto& p : *_lp)
        _view->setInView( p.first, FACE_LATERAL_LEFT, p.second);
    for ( const auto& p : _lmksM)
        _view->setInView( p.first, FACE_LATERAL_MEDIAL, p.second);
    for ( const auto& p : *_rp)
        _view->setInView( p.first, FACE_LATERAL_RIGHT, p.second);
}   // end registerViewer


bool LandmarkSet::has( int id, FaceLateral lat) const
{
    if ( !has(id))
        return false;

    bool hasLmk = false;
    switch (lat)
    {
        case FACE_LATERAL_LEFT:
            hasLmk = _lp->count(id) > 0;
            break;
        case FACE_LATERAL_MEDIAL:
            hasLmk = _lmksM.count(id) > 0;
            break;
        case FACE_LATERAL_RIGHT:
            hasLmk = _rp->count(id) > 0;
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
            lmks = _lp;
            break;
        case FACE_LATERAL_MEDIAL:
            lmks = &_lmksM;
            break;
        case FACE_LATERAL_RIGHT:
            lmks = _rp;
            break;
    }   // end switch
    return *lmks;
}   // end lateral


// private
LandmarkSet::LDMRKS& LandmarkSet::lateral( FaceLateral lat) { return const_cast<LDMRKS&>( lateral(lat));}


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
    if ( _view)
        _view->setInView(id, lat, v);
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
    Landmark* lmk = LDMKS_MAN::landmark(id);   // Check if landmark is real and deletable first
    if ( !lmk || !lmk->isDeletable())
        return false;

    const bool inleft  = _lp->count(id) > 0;
    const bool inmid   = _lmksM.count(id) > 0;
    const bool inright = _rp->count(id) > 0;

    _ids.erase(id);
    _lp->erase(id);
    _lmksM.erase(id);
    _rp->erase(id);
    _names.erase(lmk->name());
    _codes.erase(lmk->code());

    if ( _view)
    {
        if ( inleft)
            _view->removeFromView(id, FACE_LATERAL_LEFT);
        if ( inmid)
            _view->removeFromView(id, FACE_LATERAL_MEDIAL);
        if ( inright)
            _view->removeFromView(id, FACE_LATERAL_RIGHT);
    }   // end if
    return true;
}   // end erase


const cv::Vec3f* LandmarkSet::pos( int id, FaceLateral lat) const
{
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
    if ( _view)
        _view->setInView(id, lat, lateral(lat).at(id));
    return true;
}   // end translate


void LandmarkSet::translate( const cv::Vec3f& t)
{
    for ( auto& p : *_lp)
        translate( p.first, FACE_LATERAL_LEFT, t);
    for ( auto& p : _lmksM)
        translate( p.first, FACE_LATERAL_MEDIAL, t);
    for ( auto& p : *_rp)
        translate( p.first, FACE_LATERAL_RIGHT, t);
}   // end translate


// private
void LandmarkSet::updateView( int8_t lat)
{
    if ( _view)
    {
        if ( lat & FACE_LATERAL_LEFT)
        {
            for ( auto& p : *_lp)
                _view->setInView( p.first, FACE_LATERAL_LEFT, p.second);
        }   // end if

        if ( lat & FACE_LATERAL_MEDIAL)
        {
            for ( auto& p : _lmksM)
                _view->setInView( p.first, FACE_LATERAL_MEDIAL, p.second);
        }   // end if

        if ( lat & FACE_LATERAL_RIGHT)
        {
            for ( auto& p : *_rp)
                _view->setInView( p.first, FACE_LATERAL_RIGHT, p.second);
        }   // end if
    }   // end if
}   // end updateView


void LandmarkSet::transform( const cv::Matx44d& T)
{
    const RFeatures::Transformer mover(T);
    for ( auto& p : *_lp)
        mover.transform( _lp->at(p.first));
    for ( auto& p : _lmksM)
        mover.transform( _lmksM.at(p.first));
    for ( auto& p : *_rp)
        mover.transform( _rp->at(p.first));
    updateView( FACE_LATERAL_LEFT | FACE_LATERAL_MEDIAL | FACE_LATERAL_RIGHT);
}   // end transform


double LandmarkSet::translateToSurface( RFeatures::ObjModelKDTree::Ptr kdt)
{
    double sdiff = 0;
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt->model());

    cv::Vec3f fv;
    int notused, vidx;

    for ( const auto& p : *_lp)
    {
        const cv::Vec3f& v = p.second;  // Current position of landmark
        vidx = kdt->find( v);   // Closest vertex to landmark
        // Project v onto the model's surface. Choose from all the polygons connected to vertex vidx the
        // projection into the plane of a polygon that gives the smallest difference in position.
        sdiff += spfinder.find( v, vidx, notused, fv);
        _lp->at(p.first) = fv;
    }   // end for

    for ( const auto& p : _lmksM)
    {
        const cv::Vec3f& v = p.second;
        vidx = kdt->find( v);
        sdiff += spfinder.find( v, vidx, notused, fv);
        _lmksM.at(p.first) = fv;
    }   // end for

    for ( const auto& p : *_rp)
    {
        const cv::Vec3f& v = p.second;
        vidx = kdt->find( v);
        sdiff += spfinder.find( v, vidx, notused, fv);
        _rp->at(p.first) = fv;
    }   // end for

    updateView( FACE_LATERAL_LEFT | FACE_LATERAL_MEDIAL | FACE_LATERAL_RIGHT);
    return sqrt( sdiff / size());    // Average difference in reposition of landmarks
}   // end translateToSurface


void LandmarkSet::reflect()
{
    if ( _lp == &_lmksL)
    {
        _lp = &_lmksR;
        _rp = &_lmksL;
    }   // end if
    else
    {
        _lp = &_lmksL;
        _rp = &_lmksR;
    }   // end else

    updateView( FACE_LATERAL_LEFT | FACE_LATERAL_RIGHT);
}   // end reflect


// Write out the landmarks to record.
void LandmarkSet::write( PTree& lnodes) const
{
    PTree& llat = lnodes.put("LeftLateral", "");
    for ( const auto& p : *_lp)
        RFeatures::putNamedVertex( llat, LDMKS_MAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& mlat = lnodes.put("Medial", "");
    for ( const auto& p : _lmksM)
        RFeatures::putNamedVertex( mlat, LDMKS_MAN::landmark(p.first)->code().toStdString(), p.second);
    PTree& rlat = lnodes.put("RightLateral", "");
    for ( const auto& p : *_rp)
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
