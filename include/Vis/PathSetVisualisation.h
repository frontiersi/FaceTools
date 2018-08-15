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

#ifndef FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H
#define FACE_TOOLS_VIS_PATH_SET_VISUALISATION_H

#include "BaseVisualisation.h"
#include "PathSetView.h"
#include <vtkTextActor.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT PathSetVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    PathSetVisualisation( const QString &dname, const QIcon &icon);
    ~PathSetVisualisation() override;

    bool isExclusive() const override { return false;}
    bool singleModel() const override { return true;}

    bool belongs( const vtkProp*, const FaceControl*) const override;

    bool apply( const FaceControl*, const QPoint* mc=nullptr) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

    // Creates a new path view for the given path - returning one of the handles (both initially in same position).
    const PathView::Handle* addPath( const FaceControl*, int pathId);

    // Returns the handle for a path by looking for it by prop.
    const PathView::Handle* pathHandle( const FaceControl*, const vtkProp*) const;

    // Returns handles 0 or 1 for the given path.
    const PathView::Handle* pathHandle0( const FaceControl*, int pathID) const;
    const PathView::Handle* pathHandle1( const FaceControl*, int pathID) const;

    // Update the position of the handles for the given path.
    void updatePath( const FaceControl*, int pathId);

    // Set caption info to be for the given path. Caption will be anchored at
    // the given attachment point, or halfway along the path if left null.
    void setCaptions( const FaceControl*, int pathId, const cv::Vec3f* attachPoint=nullptr);

    // Show/hide the caption (distance) information.
    void setCaptionsVisible( bool v);

protected:
    void pokeTransform( const FaceControl*, const vtkMatrix4x4*) override;
    void fixTransform( const FaceControl*) override;
    void purge( const FaceControl*) override;

private:
    // The paths associated with a FaceControl.
    std::unordered_map<const FaceControl*, PathSetView*> _views;
    vtkSmartPointer<vtkCaptionActor2D> _caption; // Shared between all
    vtkSmartPointer<vtkTextActor> _text;         // Shared between all
};  // end class

}   // end namespace
}   // end namespace

#endif
