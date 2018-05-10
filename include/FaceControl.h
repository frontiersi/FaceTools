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

#ifndef FACE_TOOLS_FACE_CONTROL_H
#define FACE_TOOLS_FACE_CONTROL_H

/**
 * Container that associates a model to a FaceView (and its visualisations).
 * There can be many FaceControl instances associated with a single FaceModel.
 * Provides convenience functions for coordinating transformations between
 * the view and data models.
 */

#include "FaceTools_Export.h"
#include <opencv2/opencv.hpp>
#include <vtkMatrix4x4.h>

namespace FaceTools {
class FaceModelViewer;
class FaceModel;

namespace Vis {
class FaceView;
}   // end namespace

class FaceTools_EXPORT FaceControl
{
public:
    explicit FaceControl( FaceModel*);
    virtual ~FaceControl();

    // Transform both the data and ALL FaceControl instances attached to the data (not just this one).
    // This can be expensive in terms of data updates. For user interaction it is better to use a
    // vtkInteractor (actor style) on the desired FaceView actors, then use the final transformation
    // matrix to update the model once interactive movement of the vtkActor is finished (e.g. at the end of a mouse drag).
    void transform( const cv::Matx44d&);

    // Hard transform the view models without changing the data (FaceModel).
    // Afterwards, the user transform for the view will be the identity matrix.
    void transformView( const vtkMatrix4x4*);

    // After moving the actors in the FaceView via interaction, fix the position of this view and
    // every view attached to this FaceControl's associated FaceModel. This function calls transform
    // with the user transform returned from the associated FaceView. On return, the user transform
    // for all associated views will be the identity matrix.
    void transformFromView();

    // Convenience function to get/set viewer this FaceControl uses.
    FaceModelViewer* viewer() const;
    void setViewer( FaceModelViewer*);

    FaceModel* data() const { return _fdata;}
    Vis::FaceView* view() const { return _fview;}

private:
    FaceModel *_fdata;
    Vis::FaceView *_fview;

    FaceControl( const FaceControl&);     // No copy
    void operator=( const FaceControl&);  // No copy
};  // end class

}   // end namespace
    
#endif
