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

#ifndef FACE_TOOLS_FACE_MODEL_H
#define FACE_TOOLS_FACE_MODEL_H

#include "PathSet.h"
#include "LandmarkSet.h"
#include "FaceControlSet.h"
#include <ObjModelTools.h>   // RFeatures
#include <QReadWriteLock>

namespace FaceTools {
class FaceControl;

class FaceTools_EXPORT FaceModel
{
public:
    explicit FaceModel( RFeatures::ObjModelInfo::Ptr);
    FaceModel();

    // Use these read/write locks before accessing or writing to this FaceModel.
    void lockForWrite();
    void lockForRead() const;
    void unlock() const;    // Call after done with read or write locks.

    // Update with new data. Returns false iff a null object is passed in or the wrapped ObjModel is not a
    // valid 2D manifold. If parameter NULL, update with existing data (which is presumed to have changed).
    bool update( RFeatures::ObjModelInfo::Ptr=nullptr);

    // For making linear changes to the model that can be expressed using a matrix.
    // Transform the model, the orientation, and the landmarks using the given matrix.
    void transform( const cv::Matx44d&);

    // Use this function to access the model for making direct changes. After making
    // changes, call update to ensure that updates propagate through. If making changes
    // to the wrapped ObjModel, ensure that ObjModelInfo::reset is called before update.
    RFeatures::ObjModelInfo::Ptr info() const { return _minfo;}

    // Get the KD-tree - DO NOT MAKE CHANGES TO IT DIRECTLY!
    RFeatures::ObjModelKDTree::Ptr kdtree() const { return _kdtree;}

    // Returns boundary values for each model component as [xmin,xmax,ymin,ymax,zmin,zmax].
    const std::vector<cv::Vec6d>& bounds() const { return _cbounds;}

    // Set/get orientation of the data.
    void setOrientation( const RFeatures::Orientation &o) { _orientation = o; setSaved(false);}
    const RFeatures::Orientation& orientation() const { return _orientation;}

    // Landmarks.
    LandmarkSet::Ptr landmarks() const { return _landmarks;}    // CALL setSaved(false) AFTER UPDATING!
    // Paths.
    PathSet::Ptr paths() const { return _paths;}    // CALL setSaved(false) AFTER UPDATING!

    // Set/get description of data.
    void setDescription( const std::string& d) { _description = d; setSaved(false);}
    const std::string& description() const { return _description;}

    // Set/get source of data.
    void setSource( const std::string& s) { _source = s; setSaved(false);}
    const std::string& source() const { return _source;}

    const FaceControlSet& faceControls() const { return _fcs;}

    // Set/get if this model needs saving.
    bool isSaved() const { return _saved;}
    void setSaved( bool s=true) { _saved = s;}

    bool hasMetaData() const;

    // Convenience function to update renderers on all associated FaceControls.
    void updateRenderers() const;

    // Translate the given point to the surface of this model. First finds the
    // closest point on the surface using the internal kd-tree.
    double translateToSurface( cv::Vec3f&) const;

private:
    bool _saved;
    std::string _description;   // Long form description
    std::string _source;        // Data source info
    RFeatures::Orientation _orientation;
    FaceTools::LandmarkSet::Ptr _landmarks;
    FaceTools::PathSet::Ptr _paths;
    RFeatures::ObjModelInfo::Ptr _minfo;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    std::vector<cv::Vec6d> _cbounds;    // Per component bounds

    mutable QReadWriteLock _mutex;
    FaceControlSet _fcs;  // Associated FaceControls
    friend class FaceControl;

    void calculateBounds();
    FaceModel( const FaceModel&) = delete;
    void operator=( const FaceModel&) = delete;
};  // end class

}   // end namespace

#endif
