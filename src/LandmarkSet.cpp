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

#include <LandmarkSet.h>
#include <Transformer.h>  // RFeatures
#include <ObjModelSurfacePointFinder.h>
using FaceTools::LandmarkSet;
using FaceTools::Landmark;
#include <algorithm>
#include <cassert>


LandmarkSet::LandmarkSet() : _sid(0) {}


LandmarkSet::LandmarkSet( const LandmarkSet& lset)
{
    (*this) = lset;
}   // end ctor


LandmarkSet& LandmarkSet::operator=( const LandmarkSet& lset)
{
    _landmarks = lset._landmarks;
    _landmarkIDs = lset._landmarkIDs;
    _names = lset._names;
    _ids = lset._ids;
    _sid = lset._sid;
    return *this;
}   // end operator=


int LandmarkSet::set( const std::string& name, const cv::Vec3f& v)
{
    int id = -1;
    if ( has(name))
    {
        _landmarks.at(name).pos = v;   // Retains existing meta data (visible, movable, deletable)
        id = _landmarks.at(name).id;
        assert( _landmarkIDs.count( id) > 0);
        assert( _ids.count( id) > 0);
    }   // end if
    else
    {
        _landmarks[name] = Landmark( _sid++, name,v);        // Add a new landmark
        _names.insert(name);
        id = _landmarks.at(name).id;
        _ids.insert(id);
        _landmarkIDs[id] = name;
    }   // end else
    return id;
}   // end set


bool LandmarkSet::set( int id, const cv::Vec3f& v)
{
    if ( !has(id))
        return false;
    get(id)->pos = v;
    return true;
}   // set


const cv::Vec3f& LandmarkSet::pos( const std::string& name) const
{
    assert( has(name));
    return get(name)->pos;
}   // end pos


const cv::Vec3f& LandmarkSet::pos( int id) const
{
    assert( has(id));
    return get(id)->pos;
}   // end pos


Landmark* LandmarkSet::get( const std::string& name) { return _landmarks.count(name) == 0 ? NULL : &_landmarks.at(name);}
Landmark* LandmarkSet::get( int id) { return _landmarkIDs.count(id) == 0 ? NULL : get( _landmarkIDs.at(id));}
const Landmark* LandmarkSet::get( const std::string& name) const { return _landmarks.count(name) == 0 ? NULL : &_landmarks.at(name);}
const Landmark* LandmarkSet::get( int id) const { return _landmarkIDs.count(id) == 0 ? NULL : get( _landmarkIDs.at(id));}


bool LandmarkSet::erase( const std::string& name)
{
    if (!has(name))
        return false;
    const int id = _landmarks.at(name).id;
    _ids.erase(id);
    _landmarkIDs.erase( id);
    _landmarks.erase(name);
    _names.erase(name);
    return true;
}   // end erase


bool LandmarkSet::erase( int id)
{
    if (!has(id))
        return false;
    return erase( _landmarkIDs.at(id));
}   // end erase


bool LandmarkSet::changeName( int id, const std::string& newname)
{
    if ( !has(id) || has(newname))
        return false;

    const std::string oldname = _landmarkIDs.at(id);
    Landmark lm = _landmarks.at( oldname);
    lm.name = newname;
    _landmarks.erase(oldname);
    _names.erase(oldname);
    _landmarks[newname] = lm;
    _names.insert(newname);
    _landmarkIDs[id] = newname;
    return true;
}   // end changeName


bool LandmarkSet::translate( const std::string& name, const cv::Vec3f& t)
{
    if (!has(name))
        return false;
    get(name)->pos += t;
    return true;
}   // end translate


bool LandmarkSet::translate( int id, const cv::Vec3f& t)
{
    if (!has(id))
        return false;
    get(id)->pos += t;
    return true;
}   // end translate


// Translate the landmarks
void LandmarkSet::translate( const cv::Vec3f& t)
{
    std::for_each( std::begin(_ids), std::end(_ids), [&](int id){ get(id)->pos += t;});
}   // end translate


void LandmarkSet::transform( const cv::Matx44d& T)
{
    const RFeatures::Transformer mover(T);
    std::for_each( std::begin(_ids), std::end(_ids), [&](int id){ mover.transform( get(id)->pos);});  // Move in place
}   // end transform


double FaceTools::translateLandmarksToSurface( const RFeatures::ObjModelKDTree& kdt, LandmarkSet &lset)
{
    double sdiff = 0;
    const RFeatures::ObjModelSurfacePointFinder spfinder( kdt.model());

    cv::Vec3f fv;
    int notused, vidx;
    for ( int lmkid : lset.ids())
    {
        const cv::Vec3f& v = lset.pos( lmkid);  // Current position of landmark
        vidx = kdt.find( v);   // Closest vertex to landmark

        // Project v onto the model's surface. Choose from all the polygons connected to vertex vidx the
        // projection into the plane of a polygon that gives the smallest difference in position.
        sdiff += spfinder.find( v, vidx, notused, fv);
        lset.set( lmkid, fv);
    }   // end foreach

    return sqrt( sdiff / lset.count());    // Average difference in reposition of landmarks
}   // end translateLandmarksToSurface


// Write out the landmarks to record.
PTree& FaceTools::operator<<( PTree& record, const LandmarkSet& lset)
{
    PTree& lnodes = record.put("landmarks","");
    auto ids = lset.ids();
    std::for_each( std::begin(ids), std::end(ids), [&](int id){ lnodes << *lset.get(id);});
    return record;
}   // end operator<<


// Read in the landmarks from record.
const PTree& FaceTools::operator>>( const PTree& record, LandmarkSet& lset)
{
    const PTree& lmks = record.get_child("landmarks");
    for ( const PTree::value_type& lvt : lmks)
    {
        if ( lvt.first == "landmark")
        {
            std::string nm = lvt.second.get<std::string>( "<xmlattr>.name");
            lset.set( nm, RFeatures::getVertex( lvt));
        }   // end if
    }   // end foreach
    return record;
}   // end operator>>
