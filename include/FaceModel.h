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

    // Add/remove FaceControl instances associated with this model
    void add( FaceControl*);
    void remove( FaceControl*);

    RFeatures::ObjModel::Ptr& model() { return _model;}
    RFeatures::Orientation& orientation() { return _orientation;}
    LandmarkSet& landmarks() { return _landmarks;}
    std::string& description() { return _description;}
    std::string& source() { return _source;}

    const RFeatures::ObjModel::Ptr model() const { return _model;}
    const RFeatures::Orientation& orientation() const { return _orientation;}
    const LandmarkSet& landmarks() const { return _landmarks;}
    const std::string& description() const { return _description;}
    const std::string& source() const { return _source;}

    RFeatures::ObjModelKDTree::Ptr& kdtree() { return _kdtree;}
    const RFeatures::ObjModelKDTree::Ptr kdtree() const { return _kdtree;}

    // Transform all the data elements in space. Also rebuilds KD-tree if set.
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
    RFeatures::ObjModelCurvatureMap::Ptr _cmap;
    RFeatures::ObjModelFaceAngleCalculator _facalc;
    RFeatures::ObjModelCurvatureMetrics::Ptr _cmetrics;
    RFeatures::ObjModelFastMarcher::Ptr _udist;
    RFeatures::ObjModelFastMarcher::Ptr _cdist;
    */

    FaceModel( const FaceModel&);               // No copy
    FaceModel& operator=( const FaceModel&);    // No copy
};  // end class

}   // end namespace

#endif
