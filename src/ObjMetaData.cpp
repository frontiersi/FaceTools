#include <ObjMetaData.h>
using FaceTools::ObjMetaData;
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
ObjMetaData::Ptr ObjMetaData::create( const std::string& mfile, const ObjModel::Ptr model)
{
    return Ptr( new ObjMetaData( mfile, model), Deleter());
}   // end create


// private
ObjMetaData::ObjMetaData( const std::string mfile)
    : _mfile(mfile)
{
}   // end ctor


// private
ObjMetaData::ObjMetaData( const std::string& mfile, const ObjModel::Ptr model)
    : _mfile(mfile)
{
    setObject( model);
}   // end ctor


void ObjMetaData::setObject( ObjModel::Ptr model)
{
    _model = model;
    _kdtree.reset();
    _curvMap.reset();
}   // end setObject


void ObjMetaData::releaseObject()
{
    setObject( ObjModel::Ptr());
}   // end releaseObject


void ObjMetaData::rebuildKDTree()
{
    _kdtree.reset();
    if ( _model)
        _kdtree = RFeatures::ObjModelKDTree::create( _model);
}   // end rebuildKDTree


const RFeatures::ObjModelKDTree::Ptr ObjMetaData::getKDTree() const
{
    if ( _kdtree == NULL)
        std::cerr << "[ERROR] FaceTools::ObjMetaData::getKDTree(): kd tree is NULL!" << std::endl;
    return _kdtree;
}   // end getKDTree


void ObjMetaData::rebuildCurvatureMap( int svidx)
{
    _curvMap.reset();
    if ( _model)
    {
        const IntSet& sfids = _model->getFaceIds( svidx);
        _curvMap = RFeatures::ObjModelCurvatureMap::create( _model, *sfids.begin());
    }   // end if
}   // end rebuildCurvatureMap


const RFeatures::ObjModelCurvatureMap::Ptr ObjMetaData::getCurvatureMap() const
{
    if ( _curvMap == NULL)
        std::cerr << "[ERROR] FaceTools::ObjMetaData::getCurvatureMap(): curvature map is NULL!" << std::endl;
    return _curvMap;
}   // end getCurvatureMap


void ObjMetaData::setLandmark( const std::string& name, const cv::Vec3f& v)
{
    _landmarks[name] = v;
}   // end setLandmark


bool ObjMetaData::hasLandmark( const std::string& name) const
{
    return _landmarks.count(name) > 0;
}   // end hasLandmark


const cv::Vec3f& ObjMetaData::getLandmark( const std::string& name) const
{
    assert( hasLandmark(name));
    return _landmarks.at(name);
}   // end getLandmark


void ObjMetaData::shiftLandmarks( const cv::Vec3f& t)
{
    std::vector<std::string> lmnames;
    getLandmarks( lmnames);
    BOOST_FOREACH ( const std::string& lmname, lmnames)
        _landmarks[lmname] = _landmarks[lmname] + t;
}   // end shiftLandmarks


// public
double ObjMetaData::shiftLandmarksToSurface()
{
    using namespace RFeatures;
    double sdiff = 0.0;

    const ObjModel::Ptr model = getObject();
    const ObjModelSurfacePointFinder spfinder( model);
    const ObjModelKDTree::Ptr kdt = getKDTree();
    assert( kdt != NULL);

    int notused;
    std::vector<std::string> lmnames;
    getLandmarks( lmnames);
    BOOST_FOREACH ( const std::string& lmname, lmnames)
    {
        const cv::Vec3f& v = getLandmark( lmname);  // Current position of landmark
        int vidx = kdt->find( v);                  // Closest vertex to landmark

        // Project v onto the model's surface. Choose from all the polygons connected to vertex vidx the
        // projection into the plane of a polygon that gives the smallest difference in position.
        cv::Vec3f fv;
        sdiff += spfinder.find( v, vidx, notused, fv);
        setLandmark( lmname, fv);
    }   // end foreach

    return sqrt( sdiff / getNumLandmarks());    // Average difference in reposition of landmarks
}   // end shiftLandmarksToSurface


void ObjMetaData::transformLandmarks( const cv::Matx44d& T)
{
    std::vector<std::string> lmnames;
    getLandmarks( lmnames);
    BOOST_FOREACH ( const std::string& lmname, lmnames)
    {
        const cv::Vec3f& v = _landmarks[lmname];
        const cv::Vec4d hv( v[0], v[1], v[2], 1);
        const cv::Vec4d nv = T * hv;
        _landmarks[lmname] = cv::Vec3f( float(nv[0]), float(nv[1]), float(nv[2]));
    }   // end foreach
}   // end transformLandmarks


void ObjMetaData::transform( const cv::Matx44d& T)
{
    const RFeatures::ObjModelMover mover(T);
    mover( _model);
    transformLandmarks(T);
}   // end transform


size_t ObjMetaData::getLandmarks( std::vector<std::string>& lmks) const
{
    lmks.resize(_landmarks.size());
    size_t i = 0;
    typedef std::pair<std::string, cv::Vec3f> LMPair;
    BOOST_FOREACH ( const LMPair& lmpair, _landmarks)
        lmks[i++] = lmpair.first;
    return i;
}   // end getLandmarks


namespace PT = boost::property_tree;


// public
void ObjMetaData::writeTo( PT::ptree& tree) const
{
    PT::ptree& topNode = tree.add("record","");
    topNode.put( "filename", _mfile);
    PT::ptree& landmarks = topNode.put( "landmarks", ""); // Landmarks node
    typedef std::pair<std::string, cv::Vec3f> LMPair;
    BOOST_FOREACH ( const LMPair& lmpair, _landmarks)
    {
        const std::string& lmname = lmpair.first;
        const cv::Vec3f& v = lmpair.second;
        PT::ptree& landmark = landmarks.add( "landmark","");
        landmark.put( "<xmlattr>.name", lmname);    // Landmark identifier
        landmark.put( "x", v[0]);
        landmark.put( "y", v[1]);
        landmark.put( "z", v[2]);
    }   // end foreach
}   // end writeTo


// public static
void ObjMetaData::readFrom( const PT::ptree& record, ObjMetaData& fd)
{
    fd.setObjectFile( record.get<std::string>( "filename"));
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
    std::vector<std::string> landmarks;
    fd.getLandmarks( landmarks);
    int maxNameLen = 0;
    BOOST_FOREACH ( const std::string& lmname, landmarks)
        maxNameLen = std::max( maxNameLen, (int)lmname.size());
    os << "Object source: " << std::setw(maxNameLen) << fd.getObjectFile() << std::endl;
    BOOST_FOREACH ( const std::string& lmname, landmarks)
        os << std::right << std::setw(maxNameLen) << lmname << " @ " << fd.getLandmark(lmname) << std::endl;
    return os;
}   // end operator<<
