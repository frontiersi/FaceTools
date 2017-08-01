#include "FaceTools.h"
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
using RFeatures::ObjModel;


/*
void FaceTools::drawPath( const std::vector<cv::Vec3f>& path, cv::Mat& m, cv::Scalar col, int thickness)
{
    const int numPts = (int)path.size();
    std::vector<cv::Point> epts(numPts);
    for ( int i = 0; i < numPts; ++i)
        epts[i] = VISC::projectToPlane( path[i], m.size());
    const cv::Point* pts = &epts[0];
    const int numCurves = 1;
    const bool closedLoop = false;
    cv::polylines( m, &pts, &numPts, numCurves, closedLoop, col, thickness);
}   // end drawPath
*/


void FaceTools::removeParentheticalContent( std::string& s)
{
    bool removed = true;
    while ( removed)
    {
        const std::string::size_type p0 = s.find_first_of('(');
        const std::string::size_type p1 = s.find_first_of(')');
        if ( p0 == std::string::npos || p1 == std::string::npos || p1 < p0)
            removed = false;
        else
            s.replace( p0, p1-p0+1, "");
    }   // end while
    boost::algorithm::trim(s);
}   // end removeParentheticalContent


std::string FaceTools::getExtension( const std::string& fname)
{
    std::string fname2 = fname;
    boost::algorithm::trim(fname2);
    boost::filesystem::path p( fname2);
    if ( !p.has_extension())    // No extension
        return "";

    std::string ext = p.extension().string();
    assert( ext[0] == '.');
    if ( ext == ".")    // Empty extension
        return "";

    ext = ext.substr(1);
    boost::algorithm::to_lower(ext);  // Don't want preceeding period & set to lower case
    return ext;
}   // end getExtension


std::string FaceTools::getDateTimeDigits( const std::string& fname)
{
    // Find the first character of filename that's a digit
    const char sepch = '/'; // Works on Windows and Unix
    int sidx = (int)fname.find_last_of(sepch) + 1;
    for ( ; sidx < fname.size(); ++sidx)
    {
        if ( isdigit(fname[sidx]))
            break;
    }   // end for

    // Check that we have ten digits from sidx
    int numDigits = 0;
    int fidx;
    for ( fidx = sidx; fidx < fname.size(); ++fidx)
    {
        if ( numDigits == 12)
            break;
        if ( isdigit(fname[fidx]))
            numDigits++;
        else
            break;
    }   // end for

    std::string fnamesub;
    if ( numDigits != 12)
        std::cerr << "DateTime digits not found in filename!" << std::endl;
    else
        fnamesub = fname.substr( sidx, 12);

    return fnamesub;
}   // end getDateTimeDigits


long FaceTools::getDateTimeSecs( const std::string& fname)
{
    std::string digits = getDateTimeDigits( fname);
    long secs = 0;
    if ( !digits.empty())
    {
        std::istringstream iss( digits);
        iss >> secs;
    }   // end if
    return secs;
}   // end getDateTimeSecs


cv::Point2i FaceTools::scale( const cv::Point2i& p, double sizeRatio)
{
    return cv::Point2i( cvRound(p.x * sizeRatio), cvRound(p.y * sizeRatio));
}   // end scale


cv::Point2f FaceTools::scale( const cv::Point2f& p, double sizeRatio)
{
    return cv::Point2f( p.x * sizeRatio, p.y * sizeRatio);
}   // end scale


cv::Rect_<int> FaceTools::scale( const cv::Rect_<int>& r, double sizeRatio)
{
    return cv::Rect_<int>( cvRound(r.x * sizeRatio), cvRound(r.y * sizeRatio),
                           cvRound(r.width * sizeRatio), cvRound(r.height * sizeRatio));
}   // end scale


cv::Point FaceTools::calcCentre( const cv::Rect& r, double sizeRatio)
{
    return FaceTools::scale( cv::Point( r.x + r.width/2, r.y + r.height/2), sizeRatio);
}   // end calcCentre


cv::Point2f FaceTools::calcCentre( const cv::Rect_<float>& r, double sizeRatio)
{
    return FaceTools::scale( cv::Point2f( r.x + r.width/2, r.y + r.height/2), sizeRatio);
}   // end calcCentre


float FaceTools::calcDistance( const cv::Point& p0, const cv::Point& p1)
{
    return sqrtf( powf(p0.x - p1.x,2) + powf(p0.y - p1.y,2));
}   // end calcDistance


float FaceTools::calcDistance( const cv::Point2f& p0, const cv::Point2f& p1)
{
    return sqrtf( powf(p0.x - p1.x,2) + powf(p0.y - p1.y,2));
}   // end calcDistance


cv::Point FaceTools::calcMid( const cv::Point& p0, const cv::Point& p1)
{
    return cv::Point( cvRound((p0.x + p1.x)/2), cvRound((p0.y + p1.y)/2));
}   // end calcMid


cv::Point2f FaceTools::calcMid( const cv::Point2f& p0, const cv::Point2f& p1)
{
    return cv::Point2f( (p0.x + p1.x)/2, (p0.y + p1.y)/2);
}   // end calcMid


cv::Vec3f FaceTools::calcSum( const std::vector<cv::Vec3f>& vs)
{
    cv::Vec3f m(0,0,0);
    BOOST_FOREACH ( const cv::Vec3f& v, vs)
        m += v;
    return m;
}   // end calcSum


double FaceTools::calcLength( const std::vector<cv::Vec3f>& vs)
{
    const int n = (int)vs.size();
    if ( n == 0)
        return -1.0f;

    const cv::Vec3f* v = &vs[0];
    double len = 0.0;
    for ( int i = 1; i < n; ++i)
    {
        len += cv::norm( vs[i] - *v, cv::NORM_L2);
        v = &vs[i];
    }   // end for
    return len;
}   // end calcLength


cv::Vec3f FaceTools::calcMean( const std::vector<cv::Vec3f>& vs)
{
    cv::Vec3f m = calcSum(vs);
    const float s = vs.size();
    return cv::Vec3f( m[0]/s, m[1]/s, m[2]/s);
}   // end calcMean


cv::Rect_<int> FaceTools::fromProportion( const cv::Rect_<float>& b, const cv::Size2i& sz)
{
    return cv::Rect_<int>( cvRound(b.x * (sz.width-1)),
                           cvRound(b.y * (sz.height-1)),
                           cvRound(b.width * sz.width),
                           cvRound(b.height * sz.height));
}   // end fromProportion


cv::Rect_<float> FaceTools::toProportion( const cv::Rect_<int>& b, const cv::Size2i& sz)
{
    return cv::Rect_<float>( float(b.x) / (sz.width-1),
                             float(b.y) / (sz.height-1),
                             float(b.width) / sz.width,
                             float(b.height) / sz.height);
}   // end toProportion


cv::Point2i FaceTools::fromProportion( const cv::Point2f& p, const cv::Size2i& sz)
{
    return cv::Point2i( cvRound(p.x * (sz.width-1)), cvRound(p.y * (sz.height-1)));
}   // end fromProportion


cv::Point2f FaceTools::toProportion( const cv::Point2i& p, const cv::Size2i& sz)
{
    return cv::Point2f( float(p.x) / (sz.width-1), float(p.y) / (sz.height-1));
}   // end toProportion


cv::Size2i FaceTools::fromProportion(const cv::Size2f& s, const cv::Size2i& sz)
{
    return cv::Size2i( cvRound(s.width * sz.width), cvRound(s.height * sz.height));
}   // end fromProportion


cv::Size2f FaceTools::toProportion(const cv::Size2i& s, const cv::Size2i& sz)
{
    return cv::Size2f( float(s.width)/sz.width, float(s.height)/sz.height);
}   // end toProportion


cv::RotatedRect FaceTools::fromProportion( const cv::RotatedRect& r, const cv::Size2i& sz)
{
    const cv::Point2f np = fromProportion( r.center, sz);
    const cv::Size2i nsz = fromProportion( r.size, sz);
    return cv::RotatedRect( np, nsz, r.angle);
}   // end fromProportion


cv::RotatedRect FaceTools::toProportion( const cv::RotatedRect& r, const cv::Size2i& sz)
{
    const cv::Point2f np = toProportion( r.center, sz);
    const cv::Size2f nsz = toProportion( r.size, sz);
    return cv::RotatedRect( np, nsz, r.angle);
}   // end toProportion


void FaceTools::getVertices( const ObjModel::Ptr m, const std::vector<int>& uvids, std::vector<cv::Vec3f>& path)
{
    const int nvs = (int)uvids.size();
    path.resize(nvs);
    for ( int i = 0; i < nvs; ++i)
        path[i] = m->getVertex(uvids[i]);
}   // end getVertices


bool FaceTools::getVertexIndices( const ObjModel::Ptr m, const std::vector<cv::Vec3f>& vs, std::vector<int>& vidxs)
{
    const int nvs = (int)vs.size();
    vidxs.resize(nvs);
    int vidx;
    for ( int i = 0; i < nvs; ++i)
    {
        vidx = m->lookupVertexIndex(vs[i]);   // Returns -1 if can't find
        assert( vidx >= 0);
        if ( vidx < 0)
            return false;
        vidxs[i] = vidx;
    }   // end for
    return true;
}   // end getVertexIndices


void FaceTools::findNearestVertexIndices( const RFeatures::ObjModelKDTree& kdtree, const std::vector<cv::Vec3f>& vs, std::vector<int>& vidxs)
{
    const ObjModel::Ptr m = kdtree.getObject();
    const int nvs = (int)vs.size();
    vidxs.resize(nvs);
    int vidx;
    for ( int i = 0; i < nvs; ++i)
    {
        vidx = m->lookupVertexIndex(vs[i]);   // Returns -1 if can't find
        if ( vidx < 0)
            vidx = kdtree.find( vs[i]);
        assert( vidx >= 0);
        vidxs[i] = vidx;
    }   // end for
}   // end findNearestVertexIndices


cv::Mat_<byte> FaceTools::removeBlackBackground( const cv::Mat_<byte>& m)
{
    // Get the mean value of the input image without the black background
    const byte meanVal = cv::saturate_cast<byte>( cv::sum(m)[0] / cv::countNonZero(m));
    const int ncols = m.cols;
    const int nrows = m.rows;
    cv::Mat_<byte> wm = m.clone();
    for ( int i = 0; i < nrows; ++i)
    {
        byte* row = wm.ptr<byte>(i);

        for ( int j = 0; j < ncols; ++j)    // From left
        {
            if ( row[j])
                break;
            row[j] = meanVal;
        }   // end for

        for ( int j = ncols-1; j >= 0; --j) // From right
        {
            if ( row[j])
                break;
            row[j] = meanVal;
        }   // end for
    }   // end for
    return wm;
}   // end removeBlackBackground


cv::Rect FaceTools::getIntersection( const std::list<cv::Rect>& boxes)
{
    cv::Rect ibox(0,0,0,0);
    const int nidxs = (int)boxes.size();
    assert( nidxs > 0);
    if ( nidxs > 0)
    {
        std::list<cv::Rect>::const_iterator i = boxes.begin();
        ibox = *i;
        i++;
        for ( ; i != boxes.end(); ++i)
            ibox &= *i;
    }   // end if
    return ibox;
}   // end getIntersection


cv::Rect FaceTools::getUnion( const std::list<cv::Rect>& boxes)
{
    cv::Rect ubox(0,0,0,0);
    const int nidxs = (int)boxes.size();
    assert( nidxs > 0);
    if ( nidxs > 0)
    {
        std::list<cv::Rect>::const_iterator i = boxes.begin();
        ubox = *i;
        i++;
        for ( ; i != boxes.end(); ++i)
            ubox |= *i;
    }   // end if
    return ubox;
}   // end getUnion


cv::Rect FaceTools::getMean( const std::list<cv::Rect>& boxes)
{
    const int nidxs = (int)boxes.size();
    assert( nidxs > 0);
    cv::Rect mbox = *boxes.begin();
    std::list<cv::Rect>::const_iterator i = boxes.begin();
    i++;
    for ( ; i != boxes.end(); ++i)
    {
        const cv::Rect& box = *i;
        mbox.x += box.x;
        mbox.y += box.y;
        mbox.width += box.width;
        mbox.height += box.height;
    }   // end for

    mbox.x = (int)cvRound( double(mbox.x) / nidxs);
    mbox.y = (int)cvRound( double(mbox.y) / nidxs);
    mbox.width = (int)cvRound( double(mbox.width) / nidxs);
    mbox.height = (int)cvRound( double(mbox.height) / nidxs);
    return mbox;
}   // end getMean


cv::RotatedRect FaceTools::createRotatedRect( const cv::Point& p0, const cv::Point& p1, const cv::Size& sz)
{
    const cv::Vec2f vec( p1.x - p0.x, p1.y - p0.y);
    const cv::Point2f centre = (p0 + p1) * 0.5;
    const float angle = atan2f(vec[1], vec[0]);
    return cv::RotatedRect( centre, sz, 180.0 * angle/CV_PI);   // Angle in degrees!
}   // end createRotatedRect


float FaceTools::calcAngleDegs( const cv::Point2f& p0, const cv::Point2f& p1)
{
    return atan2f( p1.y - p0.y, p1.x - p0.x) * 180.0f/CV_PI;
}   // end calcAngleDegs


cv::Mat FaceTools::rotateUpright( const cv::Mat& img, const cv::RotatedRect& rr)
{
    // Centre point as an absolute point in img
    cv::Point2f cp = RFeatures::calcOffset( rr.boundingRect(), cv::Point2f(0.5,0.5));
    // Get a matrix header for the portion of the image that's actually going to be rotated
    cv::Mat srcImg = img( rr.boundingRect() & cv::Rect(0,0,img.cols,img.rows));
    // Make the centre point relative to this area
    cp.x -= rr.boundingRect().x;
    cp.y -= rr.boundingRect().y;
    // rotMat is a 2x3 matrix
    const cv::Mat_<float> rotMat = cv::getRotationMatrix2D( cp, rr.angle, 1.0);
    cv::Mat dstimg;
    cv::warpAffine( srcImg, dstimg, rotMat, srcImg.size());
    // dstimg should now be rotated, but we need the subregion given by the original
    // rotated rectangle rather than the bounding rect of the rotated rectangle.
    const cv::Rect brect( cp.x - rr.size.width/2, cp.y - rr.size.height/2, rr.size.width, rr.size.height);
    return dstimg( brect & cv::Rect(0,0,dstimg.cols,dstimg.rows));
}   // end rotateUpright


int FaceTools::findMidway( const ObjModel::Ptr model, const std::vector<int>& spidxs)
{
    const int nidxs = (int)spidxs.size();
    assert( nidxs > 0);
    if ( nidxs == 0)
        return -1;

    // Get the endpoint vertices
    const cv::Vec3f v0 = model->getVertex(spidxs[0]);
    const cv::Vec3f v1 = model->getVertex(spidxs[nidxs-1]);

    int midwayUvidx = spidxs[0];
    double minDelta = DBL_MAX;
    for ( int i = 0; i < nidxs; ++i)
    {
        const int uvidx = spidxs[i];
        const cv::Vec3f& uv = model->getVertex(uvidx);
        const double delta = pow( cv::norm(uv - v0) - cv::norm(uv - v1), 2);
        if ( delta < minDelta)
        {
            minDelta = delta;
            midwayUvidx = uvidx;
        }   // end if
    }   // end for

    return midwayUvidx;
}   // end findMidway


cv::Vec3f FaceTools::getShortestPath( const ObjModel::Ptr m, int v0, int v1, std::vector<int>& uvidxs)
{
    RFeatures::DijkstraShortestPathFinder dspf( m);
    dspf.setEndPointVertexIndices( v0, v1);
    if ( dspf.findShortestPath(uvidxs) == -1)
        return cv::Vec3f(0,0,0);
    const int mp = FaceTools::findMidway( m, uvidxs);
    return m->getVertex(mp);
}   // end getShortestPath


cv::Vec3f FaceTools::calcDirectionVectorFromBase( const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& apex)
{
    cv::Vec3f base; // Get the normalised base vector
    cv::normalize(v1 - v0, base);
    // Dot product of (apex - v0) with the normalised base vector gives the projection
    // of (apex - v0) along the base vector.
    const float d = (apex - v0).dot(base);
    // This distance d along the base from v0 gives the midpoint along the base of the triangle.
    // The direction vector whose magnitude is also the triangle's height is then simply apex-midpoint.
    const cv::Vec3f midpoint = v0 + d*base;
    return apex - midpoint;
}   // end calcDirectionVectorFromBase


int FaceTools::growOut( const ObjModel::Ptr model, const cv::Vec3f& growVec, int vi)
{
    double growth, maxGrowth;
    int ni = vi;
    do
    {
        vi = ni;
        maxGrowth = 0;
        // Find the connected vertex that maximises the distance along the growth vector from bv.
        const cv::Vec3f& bv = model->getVertex(vi);
        const IntSet& conns = model->getConnectedVertices(vi);
        BOOST_FOREACH ( const int& ci, conns)
        {
            const cv::Vec3f& cv = model->getVertex(ci);
            growth = (cv - bv).dot(growVec);
            if ( growth >= maxGrowth)
            {
                maxGrowth = growth;
                ni = ci;
            }   // end if
        }   // end foreach
    } while ( ni != vi);
    return vi;
}   // end growOut


int FaceTools::toVector( const vtkSmartPointer<vtkIdList>& vlist, std::vector<int>& vs)
{
    const int n = vlist->GetNumberOfIds();
    int j = (int)vs.size();
    vs.resize( j + n);
    for ( int i = 0; i < n; ++i, ++j)
        vs[j] = vlist->GetId(i);
    return n;
}   // end toVector
