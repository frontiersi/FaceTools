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

#include <ObjMetaData.h>
using FaceTools::ObjMetaData;
using FaceTools::Landmarks::Landmark;
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <algorithm>
using RFeatures::ObjModel;

class ObjMetaData::Deleter
{ public:
    void operator()( ObjMetaData* p) { delete p;}
};  // end class


// public static
ObjMetaData::Ptr ObjMetaData::create( const std::string mfile)
{
    return Ptr( new ObjMetaData( mfile), Deleter());
}   // end create


// public static
ObjMetaData::Ptr ObjMetaData::create( ObjModel::Ptr model)
{
    return Ptr( new ObjMetaData( "", model), Deleter());
}   // end create


// public static
ObjMetaData::Ptr ObjMetaData::create( const std::string& mfile, ObjModel::Ptr model)
{
    return Ptr( new ObjMetaData( mfile, model), Deleter());
}   // end create


ObjMetaData::Ptr ObjMetaData::copy() const
{
    return Ptr( new ObjMetaData( *this), Deleter());
}   // end copy


// private
ObjMetaData::ObjMetaData( const ObjMetaData& omd)
    : _mfile(omd._mfile), _nvec(omd._nvec), _uvec(omd._uvec)
{
    _landmarks = omd._landmarks;
    setObject( omd._model);
}   // end ctor


// private
ObjMetaData::ObjMetaData( const std::string mfile)
    : _mfile(mfile), _nvec(0,0,0), _uvec(0,0,0)
{
}   // end ctor


// private
ObjMetaData::ObjMetaData( const std::string& mfile, ObjModel::Ptr model)
    : _mfile(mfile)
{
    setObject( model);
}   // end ctor


void ObjMetaData::setObject( ObjModel::Ptr model, bool buildKD)
{
    _model.reset();
    _kdtree.reset();
    _curvMap.reset();
    if ( model)
        _model = model;
    if ( model && buildKD)
        _kdtree = RFeatures::ObjModelKDTree::create( _model);
}   // end setObject


void ObjMetaData::releaseObject()
{
    setObject( ObjModel::Ptr());
}   // end releaseObject


const RFeatures::ObjModelKDTree::Ptr ObjMetaData::getKDTree() const { return _kdtree;}


void ObjMetaData::rebuildCurvatureMap( int svidx)
{
    _curvMap.reset();
    if ( _model)
    {
        const IntSet& sfids = _model->getFaceIds( svidx);
        _curvMap = RFeatures::ObjModelCurvatureMap::create( _model, *sfids.begin());
    }   // end if
}   // end rebuildCurvatureMap


const RFeatures::ObjModelCurvatureMap::Ptr ObjMetaData::getCurvatureMap() const { return _curvMap;}


void ObjMetaData::setOrientation( const cv::Vec3f& nvec, const cv::Vec3f& uvec)
{
    _nvec = nvec;
    _uvec = uvec;
}   // end setOrientation


bool ObjMetaData::getOrientation( cv::Vec3f& nvec, cv::Vec3f& uvec) const
{
    bool hasOrientation = false;
    if ( cv::norm(_nvec) > 0)
    {
        nvec = _nvec;
        uvec = _uvec;
        hasOrientation = true;
    }   // end if
    return hasOrientation;
}   // end getOrientation


void ObjMetaData::setLandmark( const std::string& name, const cv::Vec3f& v)
{
    if ( _landmarks.count(name) > 0)
        _landmarks.at(name).pos = v;   // Retains existing meta data (visible, movable, deletable)
    else
        _landmarks[name] = Landmark(name,v);        // Add a new landmark
}   // end setLandmark


bool ObjMetaData::makeBoundaryHandles( const std::list<int>& boundary, std::vector<cv::Vec3f>& bhandles) const
{
    if ( boundary.empty())
        return false;

    cv::Vec3f nvec, uvec;
    if ( !getOrientation( nvec, uvec))
        return false;

    cv::Vec3f mp(0,0,0);
    BOOST_FOREACH ( int vidx, boundary)
        mp += _model->vtx(vidx);
    mp *= 1.0f/boundary.size();    // Middle of boundary

    // Go round the loop again checking to see which vertices are at 12 extremes
    const cv::Vec3f rvec = uvec.cross(nvec);
    std::vector<cv::Vec3f> vvs(12);
    static const double THRD = 1./3;
    vvs[0] = uvec;
    cv::normalize(  2*THRD*uvec +   THRD*rvec, vvs[1]);
    cv::normalize(    THRD*uvec + 2*THRD*rvec, vvs[2]);
    vvs[3] = rvec;
    cv::normalize(  2*THRD*rvec -   THRD*uvec, vvs[4]);
    cv::normalize(    THRD*rvec - 2*THRD*uvec, vvs[5]);
    vvs[6] = -uvec;
    cv::normalize( -2*THRD*uvec -   THRD*rvec, vvs[7]);
    cv::normalize(   -THRD*uvec - 2*THRD*rvec, vvs[8]);
    vvs[9] = -rvec;
    cv::normalize(    THRD*uvec - 2*THRD*rvec, vvs[10]);
    cv::normalize(  2*THRD*uvec -   THRD*rvec, vvs[11]);

    bhandles.resize(12);
    std::vector<double> vdps(12);   // Max dot-products in the corresponding directions
    BOOST_FOREACH ( int vidx, boundary)
    {
        const cv::Vec3f& v = _model->vtx(vidx);
        const cv::Vec3d dv = v-mp;
        for ( int i = 0; i < 12; ++i)
        {
            const double dp = dv.dot(vvs[i]);
            if ( dp > vdps[i])
            {
                vdps[i] = dp;
                bhandles[i] = v;
            }   // end if
        }   // end for
    }   // end foreach
    return true;
}   // makeBoundaryHandles


const cv::Vec3f& ObjMetaData::getLandmark( const std::string& name, bool snapToVertex) const
{
    assert( hasLandmark(name));
    const cv::Vec3f* lv = &_landmarks.at(name).pos;
    if ( snapToVertex)
    {
        const RFeatures::ObjModelKDTree::Ptr kdt = getKDTree();
        assert( kdt != NULL);
        lv = &getObject()->vtx( kdt->find( *lv));
    }   // end if
    return *lv;
}   // end getLandmark


Landmark* ObjMetaData::getLandmarkMeta( const std::string& name)
{
    return _landmarks.count(name) == 0 ? NULL : &_landmarks.at(name);
}   // end getLandmarkMeta


const Landmark* ObjMetaData::getLandmarkMeta( const std::string& name) const
{
    return _landmarks.count(name) == 0 ? NULL : &_landmarks.at(name);
}   // end getLandmarkMeta


bool ObjMetaData::hasLandmark( const std::string& name) const
{
    return _landmarks.count(name) > 0;
}   // end hasLandmark


bool ObjMetaData::deleteLandmark( const std::string& name)
{
    if (!hasLandmark(name))
        return false;
    _landmarks.erase(name);
    return true;
}   // end deleteLandmark


bool ObjMetaData::changeLandmarkName( const std::string& oldname, const std::string& newname)
{
    if (!hasLandmark(oldname))
        return false;
    if (hasLandmark(newname))
        return false;

    const Landmark& oldlm = _landmarks.at(oldname);
    _landmarks[newname] = Landmark( newname, oldlm.pos, oldlm.visible, oldlm.movable, oldlm.deletable);
    return deleteLandmark(oldname);
}   // end changeLandmarkName


bool ObjMetaData::shiftLandmark( const std::string& name, const cv::Vec3f& t)
{
    if (!hasLandmark(name))
        return false;
    _landmarks[name].pos = _landmarks[name].pos + t;
    return true;
}   // end shiftLandmark


void ObjMetaData::shiftLandmarks( const cv::Vec3f& t)
{
    boost::unordered_set<std::string> names;
    getLandmarks( names);
    BOOST_FOREACH ( const std::string& name, names) // Transform the landmarks
        _landmarks[name].pos = _landmarks[name].pos + t;
}   // end shiftLandmarks


// public
double ObjMetaData::shiftLandmarksToSurface()
{
    using namespace RFeatures;
    double sdiff = 0.0;

    const ObjModel::Ptr model = getObject();
    const ObjModelSurfacePointFinder spfinder( model);
    const ObjModelKDTree::Ptr kdt = getKDTree();

    cv::Vec3f fv;
    int notused;
    boost::unordered_set<std::string> lmnames;
    getLandmarks( lmnames);
    BOOST_FOREACH ( const std::string& lmname, lmnames)
    {
        const cv::Vec3f& v = getLandmark( lmname);  // Current position of landmark
        int vidx = kdt->find( v);                  // Closest vertex to landmark

        // Project v onto the model's surface. Choose from all the polygons connected to vertex vidx the
        // projection into the plane of a polygon that gives the smallest difference in position.
        sdiff += spfinder.find( v, vidx, notused, fv);
        setLandmark( lmname, fv);
    }   // end foreach

    return sqrt( sdiff / getNumLandmarks());    // Average difference in reposition of landmarks
}   // end shiftLandmarksToSurface


void ObjMetaData::transformLandmarks( const cv::Matx44d& T)
{
    const RFeatures::ObjModelMover mover(T);
    boost::unordered_set<std::string> lmnames;
    getLandmarks( lmnames);
    BOOST_FOREACH ( const std::string& lmname, lmnames)
        mover( _landmarks[lmname].pos); // In-place
}   // end transformLandmarks


void ObjMetaData::transform( const cv::Matx44d& T, bool buildKD)
{
    const RFeatures::ObjModelMover mover(T);

    // Adjust orientation first (don't want to translate - just rotate)
    cv::Vec3f nvec = _nvec;
    cv::Vec3f uvec = _uvec;
    mover.rotate(nvec);
    mover.rotate(uvec);
    mover( _model);
    setObject( _model, buildKD); // Need to rebuild KD tree and reset curvature!
    transformLandmarks(T);       // Then all landmarks
    setOrientation( nvec, uvec); // Set the orientation
}   // end transform


size_t ObjMetaData::getLandmarks( boost::unordered_set<std::string>& lmks) const
{
    size_t count = 0;
    typedef std::pair<std::string, Landmark> LMPair;
    BOOST_FOREACH ( const LMPair& lmpair, _landmarks)
    {
        lmks.insert(lmpair.first);
        count++;
    }   // end foreach
    return count;
}   // end getLandmarks


namespace PT = boost::property_tree;


void putVertex( PT::ptree& node, const cv::Vec3f& v)
{
    node.put( "x", v[0]);
    node.put( "y", v[1]);
    node.put( "z", v[2]);
}   // end putVertex


// public
void ObjMetaData::writeTo( PT::ptree& tree) const
{
    PT::ptree& topNode = tree.add("record","");
    topNode.put( "filename", _mfile);

    cv::Vec3f nvec, uvec;
    if ( getOrientation( nvec, uvec))
    {
        PT::ptree& orientation = topNode.put( "orientation", "");
        PT::ptree& normal = orientation.add( "normal","");
        putVertex( normal, nvec);
        PT::ptree& upnode = orientation.add( "up","");
        putVertex( upnode, uvec);
    }   // end if

    PT::ptree& landmarks = topNode.put( "landmarks", ""); // Landmarks node
    typedef std::pair<std::string, Landmark> LMPair;
    BOOST_FOREACH ( const LMPair& lmpair, _landmarks)
    {
        const std::string& lmname = lmpair.first;
        const cv::Vec3f& v = lmpair.second.pos;
        PT::ptree& landmark = landmarks.add( "landmark","");
        landmark.put( "<xmlattr>.name", lmname);    // Landmark identifier
        putVertex( landmark, v);
    }   // end foreach
}   // end writeTo


// public static
void ObjMetaData::readFrom( const PT::ptree& record, ObjMetaData& fd)
{
    fd.setObjectFile( record.get<std::string>( "filename"));

    cv::Vec3f nvec, uvec;
    const PT::ptree& orientation = record.get_child( "orientation");
    BOOST_FOREACH ( const PT::ptree::value_type& vtx, orientation)
    {
        if ( vtx.first == "normal")
            nvec = cv::Vec3f( vtx.second.get<float>("x"), vtx.second.get<float>("y"), vtx.second.get<float>("z"));
        else if ( vtx.first == "up")
            uvec = cv::Vec3f( vtx.second.get<float>("x"), vtx.second.get<float>("y"), vtx.second.get<float>("z"));
    }   // end foreach
    fd.setOrientation( nvec, uvec);

    const PT::ptree& lmks = record.get_child( "landmarks");
    BOOST_FOREACH ( const PT::ptree::value_type& lmk, lmks)
    {
        if ( lmk.first == "landmark")
        {
            const std::string lmname = lmk.second.get<std::string>( "<xmlattr>.name");
            const cv::Vec3f v( lmk.second.get<float>( "x"), lmk.second.get<float>("y"), lmk.second.get<float>("z"));
            fd.setLandmark( lmname, v);
        }   // end if
    }   // end foreach
}   // end readFrom


std::ostream& operator<<( std::ostream& os, const ObjMetaData& fd)
{
    cv::Vec3f nvec, uvec;
    if ( fd.getOrientation( nvec, uvec))
    {
        os << "Normal: " << nvec << std::endl;
        os << "Up:     " << uvec << std::endl;
    }   // end if
    else
        os << "Orientation not defined!" << std::endl;

    boost::unordered_set<std::string> landmarks;
    fd.getLandmarks( landmarks);
    int maxNameLen = 0;
    BOOST_FOREACH ( const std::string& lmname, landmarks)
        maxNameLen = std::max( maxNameLen, (int)lmname.size());
    os << "Object source: " << std::setw(maxNameLen) << fd.getObjectFile() << std::endl;
    os << landmarks.size() << " landmarks:" << std::endl;
    BOOST_FOREACH ( const std::string& lmname, landmarks)
        os << std::right << std::setw(maxNameLen) << lmname << " @ " << fd.getLandmark(lmname) << std::endl;
    return os;
}   // end operator<<
