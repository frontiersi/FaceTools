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

#ifndef FACE_TOOLS_MISC_FUNCTIONS_H
#define FACE_TOOLS_MISC_FUNCTIONS_H

#include "FaceTools_Export.h"
#include <ObjModelKDTree.h> // RFeatures
#include <opencv2/opencv.hpp>
#include <QString>
#include <vtkSmartPointer.h>
#include <vtkIdList.h>
#include <string>
typedef unsigned char byte;


namespace FaceTools {

// Returns the string context of the given file or an empty string on failure.
FaceTools_EXPORT std::string loadTextFromFile( const std::string& fname);

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
// points) returns a length of zero. Empty vectors return a length of zero.
FaceTools_EXPORT double calcLength( const std::vector<cv::Vec3f>&);

// Calculate and return length of list of vertices in given order. Vertices don't have to be connected!
FaceTools_EXPORT double calcLength( const RFeatures::ObjModel*, const std::vector<int>& vertices);

// Get N equidistant points into q from among vertices in path p starting at entry j. Returns the length of the path p.
// NB path p does not need to be a chain of sequentially connected vertices.
FaceTools_EXPORT double getEquidistant( const RFeatures::ObjModel*, const std::vector<int>& p, int j, int N, std::vector<int>& q);

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
FaceTools_EXPORT void getVertices( const RFeatures::ObjModel*, const std::vector<int>& uvids, std::vector<cv::Vec3f>& path);

// Get the unique vertex indices from a bunch of vertices for the given model.
// Exact keying of the vertex locations is used to lookup their indices, so the vertex positins
// given must be exactly as given in the model.
FaceTools_EXPORT bool getVertexIndices( const RFeatures::ObjModel*, const std::vector<cv::Vec3f>& vs, std::vector<int>& vidxs);

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
FaceTools_EXPORT int findMidway( const RFeatures::ObjModel*, const std::vector<int>& uvidxs);

// Gets the shortest path over the surface of the model between v0 and v1, setting the resulting
// unique vertex IDs in uvidxs, and returning the location of the vector approximately midway along
// this path between the two endpoints v0 and v1.
FaceTools_EXPORT cv::Vec3f getShortestPath( const RFeatures::ObjModel*, int v0, int v1, std::vector<int>& uvidxs);

// Given a 3D triangle identifed by the given three corner points (with base given by v1 - v0)
// derive the direction vector whose magnitude also gives the triangle's height.
FaceTools_EXPORT cv::Vec3f calcDirectionVectorFromBase( const cv::Vec3f& v0, const cv::Vec3f& v1, const cv::Vec3f& apex);

// Find, and return the index of the vertex maximally distant from svidx in the direction of growVec.
FaceTools_EXPORT int growOut( const RFeatures::ObjModel*, const cv::Vec3f& growVec, int svidx);

// Copy contents of vtkIdList (assumed to be integers) to given std::vector.
// Returns the number of elements copied in. Does not clear contents of vector first.
FaceTools_EXPORT int toVector( const vtkSmartPointer<vtkIdList>&, std::vector<int>&);

// Return contents of stream as a front/rear trimmed QString optionally in lowercase.
FaceTools_EXPORT QString getRmLine( std::istringstream&, bool lower=false);
}   // end namespace

#endif
