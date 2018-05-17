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

namespace FaceTools {
namespace Action {
class FaceAction;
}   // end namespace

class FaceControl;

class FaceTools_EXPORT FaceModel
{
public:
    FaceModel();
    virtual ~FaceModel(){}

    // Pass in a new ObjModel to make destructive changes i.e., rebuilding all info including
    // boundary and component info. New ObjModels are cleaned before setting. If the model
    // cannot be cleaned, false is returned and no changes are made. If no ObjModel is
    // supplied, the existing model is presumed to have been changed and data are repropagated
    // internally. Surface data are updated (including landmarks and KD-tree), but model
    // orientation is not (must be set separately).
    bool updateData( RFeatures::ObjModel::Ptr=NULL);

    // For making linear changes to the model that can be expressed using a matrix.
    // Transform the model, the orientation, and the landmarks using the given matrix
    // and then propagate changes via an internal call to updateData.
    void transform( const cv::Matx44d&);

    RFeatures::ObjModel::Ptr model() const { return _minfo->model();}
    const RFeatures::ObjModel* cmodel() const { return _minfo->model().get();}

    // Get constant references to the model's KD-tree and info.
    const RFeatures::ObjModelKDTree& kdtree() const;
    const RFeatures::ObjModelInfo& info() const;

    LandmarkSet& landmarks() { return _landmarks;}  // For making modifications
    const LandmarkSet& landmarks() const { return _landmarks;}

    // Set/get orientation of the data.
    void setOrientation( const RFeatures::Orientation& o) { _orientation = o;}
    const RFeatures::Orientation& orientation() const { return _orientation;}

    // Set/get description of data.
    void setDescription( const std::string& d) { _description = d;}
    const std::string& description() const { return _description;}

    const std::string& source() const { return _source;}
    void setSource( const std::string& s) { _source = s;}

private:
    std::string _description;   // Long form description
    std::string _source;        // Data source info
    FaceTools::LandmarkSet _landmarks;
    RFeatures::Orientation _orientation;
    RFeatures::ObjModelInfo::Ptr _minfo;
    RFeatures::ObjModelKDTree::Ptr _kdtree;
    std::unordered_set<FaceControl*> _fcs;  // Associated FaceControls
    bool _flagViewUpdate;

    friend class FaceControl;
    friend class Action::FaceAction;
    FaceModel( const FaceModel&);               // No copy
    FaceModel& operator=( const FaceModel&);    // No copy
};  // end class

}   // end namespace

#endif
