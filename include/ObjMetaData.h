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

#ifndef FACE_TOOLS_OBJ_META_DATA_H
#define FACE_TOOLS_OBJ_META_DATA_H

#include "FaceTools_Export.h"
#include <ObjModelTools.h>       // RFeatures
#include <boost/property_tree/ptree.hpp>

namespace FaceTools
{

class FaceTools_EXPORT ObjMetaData
{
public:
    typedef boost::shared_ptr<ObjMetaData> Ptr;
    static Ptr create( const std::string mfile="");
    static Ptr create( const std::string& mfile, const RFeatures::ObjModel::Ptr m);

    void setObject( RFeatures::ObjModel::Ptr m);    // Does NOT build the underlying KD-tree and curvature map, but sets them to NULL.
    RFeatures::ObjModel::Ptr getObject() const { return _model;}

    void releaseObject();   // Release reference to internal object and associated data structures.

    void rebuildKDTree();   // Call after making changes to the object.
    const RFeatures::ObjModelKDTree::Ptr getKDTree() const;

    // Build the curvature map against the currently object. Normals will be defined on polygons to have consistent
    // adjacent direction starting calculation with those attached to vidx. The normals for these starting polygons
    // have a direction chosen so that their dot product with the +Z unit vector is positive.
    void rebuildCurvatureMap( int vidx);    // NB ALSO RESETS THE REGISTRATION FACE!
    const RFeatures::ObjModelCurvatureMap::Ptr getCurvatureMap() const;

    void setObjectFile( const std::string& mfile) { _mfile = mfile;}
    const std::string& getObjectFile() const { return _mfile;}

    void setLandmark( const std::string& name, const cv::Vec3f& v);
    const cv::Vec3f& getLandmark( const std::string& name) const;
    size_t getLandmarks( std::vector<std::string>& names) const;    // Get all of the landmark names into the provided vector
    size_t getNumLandmarks() const { return _landmarks.size();}
    bool hasLandmark( const std::string& name) const;
    bool hasLandmarks() const { return !_landmarks.empty();}

    void shiftLandmarks( const cv::Vec3f&);         // Add translation vector v to all landmarks.
    void transformLandmarks( const cv::Matx44d&);   // Transform all landmarks according to the given matrix.
    void transform( const cv::Matx44d&);            // Transform both the model and the landmarks.

    // Returns the mean positional difference between the existing and the new landmark positions.
    double shiftLandmarksToSurface();   // Uses RFeatures::ObjModelSurfacePointFinder

    void writeTo( boost::property_tree::ptree&) const;      // Write ObjMetaData as XML output
    static void readFrom( const boost::property_tree::ptree&, ObjMetaData&); // Read XML metadata into a ObjMetaData object.

private:
    std::string _mfile;    // Origin info (filename, or description)
    boost::unordered_map<std::string, cv::Vec3f> _landmarks;
    RFeatures::ObjModel::Ptr _model;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    RFeatures::ObjModelCurvatureMap::Ptr _curvMap;

    ObjMetaData( const ObjMetaData&);     // No copy
    void operator=( const ObjMetaData&);  // No copy
    explicit ObjMetaData( const std::string mfile="");
    ObjMetaData( const std::string& mfile, const RFeatures::ObjModel::Ptr m);   // Does NOT build the KD-tree
    class Deleter;
};  // end class

}   // end namespace

FaceTools_EXPORT std::ostream& operator<<( std::ostream& os, const FaceTools::ObjMetaData& fd);

#endif
