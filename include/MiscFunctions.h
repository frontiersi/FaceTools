#ifndef FACE_TOOLS_MISC_FUNCTIONS_H
#define FACE_TOOLS_MISC_FUNCTIONS_H

#include "FaceTools.h"
#include <vtkSmartPointer.h>
#include <vtkIdList.h>


namespace FaceTools
{

/*
// Draw path on image m, which is assumed to have same relative dimensions as the viewer.
void drawPath( const std::vector<cv::Vec3f>&, cv::Mat& m,
               cv::Scalar colour=cv::Scalar(255,255,255), int thickness=1);
*/

// Removes all occurances of "(*)" in s (where * is any character).
FaceTools_EXPORT void removeParentheticalContent( std::string& s);

// Returns the extension (without dot and in lower case) from fname.
// Returns an empty string if fname has no extension or an empty extension (ends with a dot).
FaceTools_EXPORT std::string getExtension( const std::string& fname);

FaceTools_EXPORT std::string getDateTimeDigits( const std::string&); // Get the first 10 digits as a string from given string.

FaceTools_EXPORT long getDateTimeSecs( const std::string&);

FaceTools_EXPORT cv::Point calcCentre( const cv::Rect&, double scaleFactor=1.0);

FaceTools_EXPORT cv::Point2f calcCentre( const cv::Rect_<float>&, double scaleFactor=1.0);
FaceTools_EXPORT float calcDistance( const cv::Point&, const cv::Point&);
FaceTools_EXPORT float calcDistance( const cv::Point2f&, const cv::Point2f&);
FaceTools_EXPORT cv::Point calcMid( const cv::Point&, const cv::Point&);
FaceTools_EXPORT cv::Point2f calcMid( const cv::Point2f&, const cv::Point2f&);

FaceTools_EXPORT cv::Vec3f calcSum( const std::vector<cv::Vec3f>&);
FaceTools_EXPORT cv::Vec3f calcMean( const std::vector<cv::Vec3f>&);
// Calculates the sum over the points specified. A single point (or multiple same
// points) returns a length of zero. A length of -1 is returned if the supplied vector is empty.
FaceTools_EXPORT double calcLength( const std::vector<cv::Vec3f>&);

FaceTools_EXPORT cv::Point2i scale( const cv::Point2i&, double scaleFactor);
FaceTools_EXPORT cv::Point2f scale( const cv::Point2f&, double scaleFactor);
FaceTools_EXPORT cv::Rect scale( const cv::Rect&, double scaleFactor);

// Given fixed integer sizes, convert to and from point and rectangle proportions.
FaceTools_EXPORT cv::Point2i fromProportion( const cv::Point2f&, const cv::Size2i&);
FaceTools_EXPORT cv::Size2i fromProportion( const cv::Size2f&, const cv::Size2i&);
FaceTools_EXPORT cv::Rect_<int> fromProportion( const cv::Rect_<float>&, const cv::Size2i&);
FaceTools_EXPORT cv::RotatedRect fromProportion( const cv::RotatedRect&, const cv::Size2i&);

FaceTools_EXPORT cv::Point2f toProportion( const cv::Point2i&, const cv::Size2i&);
FaceTools_EXPORT cv::Size2f toProportion( const cv::Size2i&, const cv::Size2i&);
FaceTools_EXPORT cv::Rect_<float> toProportion( const cv::Rect_<int>&, const cv::Size2i&);
FaceTools_EXPORT cv::RotatedRect toProportion( const cv::RotatedRect&, const cv::Size2i&);

// Get the vertices/normals from the unique vertex index ids specified.
FaceTools_EXPORT void getVertices( const RFeatures::ObjModel::Ptr, const std::vector<int>& uvids, std::vector<cv::Vec3f>& path);

// Get the unique vertex indices from a bunch of vertices for the given model.
// Exact keying of the vertex locations is used to lookup their indices, so the vertex positins
// given must be exactly as given in the model.
FaceTools_EXPORT bool getVertexIndices( const RFeatures::ObjModel::Ptr, const std::vector<cv::Vec3f>& vs, std::vector<int>& vidxs);

// As getVertexIndices but looks for nearest vertices if they cannot be found directly via hash lookup.
FaceTools_EXPORT void findNearestVertexIndices( const RFeatures::ObjModelKDTree&, const std::vector<cv::Vec3f>& vs, std::vector<int>& vidxs);

// Replace any black in m with the mean value of m not counting the black (0) pixels
FaceTools_EXPORT cv::Mat_<byte> removeBlackBackground( const cv::Mat_<byte>& m);

FaceTools_EXPORT cv::Rect getUnion( const std::list<cv::Rect>&);
FaceTools_EXPORT cv::Rect getIntersection( const std::list<cv::Rect>&);
FaceTools_EXPORT cv::Rect getMean( const std::list<cv::Rect>&);

// Given two points, use the angle between them and the point equidistant from them
// to define and return a rotated rectangle. The size is defined separately.
FaceTools_EXPORT cv::RotatedRect createRotatedRect( const cv::Point& p0, const cv::Point& p1, const cv::Size& sz);

FaceTools_EXPORT float calcAngleDegs( const cv::Point2f& p0, const cv::Point2f& p1);
FaceTools_EXPORT cv::Mat rotateUpright( const cv::Mat& img, const cv::RotatedRect& rr);

// Find the entry in uvidxs (giving unique vertex indices into omodel) that
// is roughly equidistant from the endpoints of the path. Entries in uvidxs
// do not have to form a connected path over the surface of the model.
FaceTools_EXPORT int findMidway( const RFeatures::ObjModel::Ptr, const std::vector<int>& uvidxs);

// Gets the shortest path over the surface of the model between v0 and v1, setting the resulting
// unique vertex IDs in uvidxs, and returning the location of the vector approximately midway along
// this path between the two endpoints v0 and v1.
FaceTools_EXPORT cv::Vec3f getShortestPath( const RFeatures::ObjModel::Ptr, int v0, int v1, std::vector<int>& uvidxs);

// Given a 3D triangle identifed by the given three corner points (with base given by v1 - v0)
// derive the direction vector whose magnitude also gives the triangle's height.
FaceTools_EXPORT cv::Vec3f calcDirectionVectorFromBase( const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& apex);

// Find, and return the index of the vertex maximally distant from svidx in the direction of growVec.
FaceTools_EXPORT int growOut( const RFeatures::ObjModel::Ptr, const cv::Vec3f& growVec, int svidx);

// Copy contents of vtkIdList (assumed to be integers) to given std::vector.
// Returns the number of elements copied in. Does not clear contents of vector first.
FaceTools_EXPORT int toVector( const vtkSmartPointer<vtkIdList>&, std::vector<int>&);

}   // end namespace

#endif
