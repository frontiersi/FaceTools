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

#include "LandmarkSet.h"
#include <ObjModelTools.h>   // RFeatures
#include <Orientation.h>     // RFeatures

namespace FaceTools {
class FaceControl;

class FaceTools_EXPORT FaceModel
{
public:
    FaceModel(){}
    virtual ~FaceModel(){}

    // Set/get the model data. Ensure setModel is called after updating the model.
    // It is not sufficient to update the model through the returned pointer without
    // calling setModel since this function updates the KD-tree and the positions
    // of other data that depend upon model surface coordinates.
    void setModel( RFeatures::ObjModel::Ptr);
    RFeatures::ObjModel::Ptr model() const { return _model;}
    const RFeatures::ObjModelKDTree::Ptr kdtree() const { return _kdtree;}

    // Set/get orientation of the data.
    void setOrientation( const RFeatures::Orientation& o) { _orientation = o;}
    const RFeatures::Orientation& orientation() const { return _orientation;}
    RFeatures::Orientation& orientation() { return _orientation;}   // Update orientation in-place

    LandmarkSet& landmarks() { return _landmarks;}  // For making modifications
    const LandmarkSet& landmarks() const { return _landmarks;}

    // Set/get description of data.
    void setDescription( const std::string& d) { _description = d;}
    const std::string& description() const { return _description;}

    const std::string& source() const { return _source;}
    void setSource( const std::string& s) { _source = s;}

    // Transform all the data by the given matrix and then transform all associated views.
    void transform( const cv::Matx44d&);

private:
    std::string _description;   // Long form description
    std::string _source;        // Data source info
    RFeatures::Orientation _orientation;
    FaceTools::LandmarkSet _landmarks;
    RFeatures::ObjModel::Ptr _model;
    std::unordered_set<FaceControl*> _fcs;  // FaceControl instances associated with this model.
    RFeatures::ObjModelKDTree::Ptr _kdtree;

    /*
    RFeatures::ObjModelFaceAngleCalculator _facalc;
    RFeatures::ObjModelFastMarcher::Ptr _udist;
    RFeatures::ObjModelFastMarcher::Ptr _cdist;
    */
    void updateData( RFeatures::ObjModel::Ptr);
    friend class FaceControl;
    FaceModel( const FaceModel&);               // No copy
    FaceModel& operator=( const FaceModel&);    // No copy
};  // end class

}   // end namespace

#endif
