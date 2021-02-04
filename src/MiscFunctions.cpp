/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <MiscFunctions.h>
#include <rimg/FeatureUtils.h>
#include <r3d/AStarSearch.h>
#include <QApplication>
#include <QTextStream>
#include <QScreen>
#include <QString>
#include <QFile>
#include <algorithm>
using r3d::Mesh;
using r3d::Vec3f;
using FaceTools::byte;


QString FaceTools::loadTextFromFile( const QString& fname)
{
    QString contents;
    QFile f(fname);
    if ( f.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream in(&f);
        contents = in.readAll();
    }   // end if
    else
        std::cerr << "[ERROR] FaceTools::loadTextFromFile: Unable to load from file " << fname.toStdString() << std::endl;
    return contents;
}   // end loadTextFromFile


QTemporaryFile* FaceTools::writeToTempFile( const QString& rfile)
{
    QTemporaryFile *tmp = new QTemporaryFile;
    if ( !tmp->open())
    {
        delete tmp;
        return nullptr;
    }   // end if

    QFile file(rfile);
    if ( file.open(QIODevice::ReadOnly))
        tmp->write( file.readAll());
    else
    {
        delete tmp;
        return nullptr;
    }   // end 
    tmp->close();

    return tmp;
}   // end writeToTempFile


/*
void FaceTools::drawPath( const std::vector<Vec3f>& path, cv::Mat& m, cv::Scalar col, int thickness)
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


std::string FaceTools::getDateTimeDigits( const std::string& fname)
{
    // Find the first character of filename that's a digit
    const char sepch = '/'; // Works on Windows and Unix
    int sidx = (int)fname.find_last_of(sepch) + 1;
    for ( ; sidx < (int)fname.size(); ++sidx)
    {
        if ( isdigit(fname[sidx]))
            break;
    }   // end for

    // Check that we have ten digits from sidx
    int numDigits = 0;
    int fidx;
    for ( fidx = sidx; fidx < (int)fname.size(); ++fidx)
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


QString FaceTools::posString( const QString prefix, const Vec3f& pos, int fw)
{
    const char rep = 'f';
    const int dp = 2;
    return QString( "%1 position: %2 X, %3 Y, %4 Z").arg(prefix).arg( pos[0], fw, rep, dp).arg( pos[1], fw, rep, dp).arg( pos[2], fw, rep, dp);
}   // end posString


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


cv::Point2i FaceTools::scale( const cv::Point2i& p, float sizeRatio)
{
    return cv::Point2i( cvRound(p.x * sizeRatio), cvRound(p.y * sizeRatio));
}   // end scale


cv::Point2f FaceTools::scale( const cv::Point2f& p, float sizeRatio)
{
    return cv::Point2f( p.x * sizeRatio, p.y * sizeRatio);
}   // end scale


cv::Rect_<int> FaceTools::scale( const cv::Rect_<int>& r, float sizeRatio)
{
    return cv::Rect_<int>( cvRound(r.x * sizeRatio), cvRound(r.y * sizeRatio),
                           cvRound(r.width * sizeRatio), cvRound(r.height * sizeRatio));
}   // end scale


cv::Point FaceTools::calcCentre( const cv::Rect& r, float sizeRatio)
{
    return FaceTools::scale( cv::Point( r.x + r.width/2, r.y + r.height/2), sizeRatio);
}   // end calcCentre


cv::Point2f FaceTools::calcCentre( const cv::Rect_<float>& r, float sizeRatio)
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


Vec3f FaceTools::calcSum( const std::vector<Vec3f>& vs)
{
    Vec3f m = Vec3f::Zero();
    std::for_each( std::begin(vs), std::end(vs), [&](const Vec3f& v){ m += v;});
    return m;
}   // end calcSum


float FaceTools::calcLength( const std::vector<Vec3f>& vs)
{
    if ( vs.empty())
        return 0;

    const size_t n = vs.size();
    const Vec3f* pv = &vs[0];
    float len = 0;
    for ( size_t i = 1; i < n; ++i)
    {
        len += ( vs[i] - *pv).norm();
        pv = &vs[i];
    }   // end for
    return len;
}   // end calcLength


float FaceTools::calcLength( const Mesh* model, const std::vector<int>& vidxs)
{
    float len = 0;
    int pvidx = vidxs.front();
    for ( int vidx : vidxs)
    {
        len += ( model->vtx(vidx) - model->vtx(pvidx)).norm();
        pvidx = vidx;
    }   // end foreach
    return len;
}   // end calcLength


float FaceTools::getEquidistant( const Mesh* model, const std::vector<int>& gpath, int j, int H, std::vector<int>& ev)
{
    assert( H > 0);
    const float glen = calcLength( model, gpath);
    const float gstep = glen / H;

    const size_t n = gpath.size();
    assert( j >= 0 && size_t(j) < n);

    int m = -1;
    ev.resize(size_t(H));
    Vec3f pv = model->vtx(gpath[size_t(j)]);    // Previous vertex
    Vec3f v = pv;
    float gsum = 0;

    for ( size_t i = 0; i < n; ++i)
    {
        v = model->vtx( gpath[size_t(j)]);
        gsum += (v - pv).norm();
        if ( int( gsum / gstep) > m)
        {
            m = (m+1) % H;
            ev[size_t(m)] = gpath[size_t(j)];
        }   // end if

        j = (j+1) % n;  // Next vertex
        pv = v;
    }   // end for

    return glen;
}   // end getEquidistant


Vec3f FaceTools::calcMean( const std::vector<Vec3f>& vs)
{
    Vec3f m = calcSum(vs);
    const float s = vs.size();
    return Vec3f( m[0]/s, m[1]/s, m[2]/s);
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


void FaceTools::getVertices( const Mesh* m, const std::vector<int>& uvids, std::vector<Vec3f>& path)
{
    const size_t nvs = uvids.size();
    path.resize(nvs);
    for ( size_t i = 0; i < nvs; ++i)
        path[i] = m->vtx(uvids[i]);
}   // end getVertices

/*
bool FaceTools::getVertexIndices( const Mesh* m, const std::vector<Vec3f>& vs, std::vector<int>& vidxs)
{
    const size_t nvs = vs.size();
    vidxs.resize(nvs);
    int vidx;
    for ( size_t i = 0; i < nvs; ++i)
    {
        vidx = m->lookupVertex(vs[i]);   // Returns -1 if can't find
        assert( vidx >= 0);
        if ( vidx < 0)
            return false;
        vidxs[i] = vidx;
    }   // end for
    return true;
}   // end getVertexIndices

void FaceTools::findNearestVertexIndices( const Mesh& m, const MeshKDTree& kdt, const std::vector<Vec3f>& vs, std::vector<int>& vidxs)
{
    const size_t nvs = vs.size();
    vidxs.resize(nvs);
    int vidx;
    for ( size_t i = 0; i < nvs; ++i)
    {
        vidx = m.lookupVertex(vs[i]);   // Returns -1 if can't find
        if ( vidx < 0)
            vidx = kdt.find( vs[i]);
        assert( vidx >= 0);
        vidxs[i] = vidx;
    }   // end for
}   // end findNearestVertexIndices
*/


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
    const size_t nidxs = boxes.size();
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
    const size_t nidxs = boxes.size();
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
    const size_t nidxs = boxes.size();
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

    mbox.x = int(cvRound( float(mbox.x) / nidxs));
    mbox.y = int(cvRound( float(mbox.y) / nidxs));
    mbox.width = int(cvRound( float(mbox.width) / nidxs));
    mbox.height = int(cvRound( float(mbox.height) / nidxs));
    return mbox;
}   // end getMean


cv::RotatedRect FaceTools::createRotatedRect( const cv::Point& p0, const cv::Point& p1, const cv::Size& sz)
{
    const cv::Vec2f vec( p1.x - p0.x, p1.y - p0.y);
    const cv::Point2f centre = (p0 + p1) * 0.5;
    const float angle = atan2f( vec[1], vec[0]);
    return cv::RotatedRect( centre, sz, float(180.0 * angle/EIGEN_PI));   // Angle in degrees!
}   // end createRotatedRect


float FaceTools::calcAngleDegs( const cv::Point2f& p0, const cv::Point2f& p1)
{
    return atan2f( p1.y - p0.y, p1.x - p0.x) * float(180.0/EIGEN_PI);
}   // end calcAngleDegs


cv::Mat FaceTools::rotateUpright( const cv::Mat& img, const cv::RotatedRect& rr)
{
    // Centre point as an absolute point in img
    cv::Point2f cp = rimg::calcOffset( rr.boundingRect(), cv::Point2f(0.5,0.5));
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


int FaceTools::findMidway( const Mesh& model, const std::vector<int>& spidxs)
{
    const int nidxs = (int)spidxs.size();
    assert( nidxs > 0);
    if ( nidxs == 0)
        return -1;

    // Get the endpoint vertices
    const Vec3f& v0 = model.vtx(spidxs[0]);
    const Vec3f& v1 = model.vtx(spidxs[nidxs-1]);

    int midwayUvidx = spidxs[0];
    float minDelta = FLT_MAX;
    for ( int i = 0; i < nidxs; ++i)
    {
        const int uvidx = spidxs[i];
        const Vec3f& uv = model.vtx(uvidx);
        const float delta = pow( (uv - v0).norm() - (uv - v1).norm(), 2);
        if ( delta < minDelta)
        {
            minDelta = delta;
            midwayUvidx = uvidx;
        }   // end if
    }   // end for

    return midwayUvidx;
}   // end findMidway


Vec3f FaceTools::getShortestPath( const Mesh& m, int v0, int v1, std::vector<int>& uvidxs)
{
    r3d::AStarSearch dspf( m);
    dspf.setEndPointVertexIndices( v0, v1);
    if ( dspf.findShortestPath(uvidxs) == -1)
        return Vec3f(0,0,0);
    const int mp = FaceTools::findMidway( m, uvidxs);
    return m.vtx(mp);
}   // end getShortestPath


Vec3f FaceTools::calcDirectionVectorFromBase( const Vec3f& v0, const Vec3f& v1, const Vec3f& apex)
{
    Vec3f base = v1 - v0; // Get the normalised base vector
    base.normalize();
    // Dot product of (apex - v0) with the normalised base vector gives the projection
    // of (apex - v0) along the base vector.
    const float d = (apex - v0).dot(base);
    // This distance d along the base from v0 gives the midpoint along the base of the triangle.
    // The direction vector whose magnitude is also the triangle's height is then simply apex-midpoint.
    const Vec3f midpoint = v0 + d*base;
    return apex - midpoint;
}   // end calcDirectionVectorFromBase


int FaceTools::growOut( const Mesh& model, const Vec3f& growVec, int vi)
{
    float growth, maxGrowth;
    int ni = vi;
    do
    {
        vi = ni;
        maxGrowth = 0;
        // Find the connected vertex that maximises the distance along the growth vector from bv.
        const Vec3f& bv = model.vtx(vi);
        const IntSet& conns = model.cvtxs(vi);
        for ( int ci : conns)
        {
            const Vec3f& cv = model.vtx(ci);
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
    const int n = int(vlist->GetNumberOfIds());
    int j = int(vs.size());
    vs.resize( size_t(j + n));
    for ( int i = 0; i < n; ++i, ++j)
        vs[size_t(j)] = int(vlist->GetId(i));
    return n;
}   // end toVector


QString FaceTools::getRmLine( std::istringstream& is, bool lower)
{
    std::string ln;
    std::getline( is, ln);
    QString qln = QString(ln.c_str()).trimmed();
    if ( lower)
        qln = qln.toLower();
    return qln;
}   // end getRmLine


void FaceTools::positionWidgetToSideOfParent( QWidget *w)
{
    assert( w->parentWidget());
    QRect prct = w->parentWidget()->geometry();
    QRect rct = w->geometry();
    const int swidth = qApp->primaryScreen()->geometry().width();
    const bool moreSpaceOnRight = prct.x() < (swidth - prct.x() - prct.width());
    const int newLPos = std::max( 0, prct.x() - rct.width());
    const int newRPos = std::min( swidth - rct.width(), prct.x() + prct.width());
    rct.moveTo( QPoint( moreSpaceOnRight ? newRPos : newLPos, prct.y()));
    w->setGeometry( rct);
}   // end positionWidgetToSideOfParent
