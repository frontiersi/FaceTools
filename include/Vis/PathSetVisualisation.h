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

    bool singleModel() const override { return true;}
    bool belongs( const vtkProp*, const FV*) const override;
    bool isAvailable( const FM*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    void remove( FV*) override;

    // Creates and shows the given path across all FaceViews for the FaceModel.
    void addPath( const FM*, int pathId);

    // Update the position of the handles for the given path.
    void updatePath( const FM*, int pathId);

    // Refresh visualisation of paths for the given FaceModel.
    void refresh( const FM*);

    // Returns the handle for a path by looking for it by prop.
    const PathView::Handle* pathHandle( const FV*, const vtkProp*) const;
    // Returns handles 0 or 1 for the given path.
    const PathView::Handle* pathHandle0( const FV*, int pathID) const;
    const PathView::Handle* pathHandle1( const FV*, int pathID) const;

    // Set caption info name, Euclidean length and path sum.
    // Caption will be displayed at render coordinates x,y.
    void setCaptions( const std::string&, double elen, double psum, int x, int y);

    // Set the point in space at which the caption should be attached.
    void setCaptionAttachPoint( const cv::Vec3f&);

    // Show/hide the captions.
    void setCaptionsVisible( bool v);

    void pokeTransform( const FV*, const vtkMatrix4x4*) override;
    void fixTransform( const FV*) override;

    static std::string CAPTION_LENGTH_METRIC;   // Defaults to "mm"

private:
    // The paths associated with a FV.
    std::unordered_map<const FV*, PathSetView*> _views;
    vtkSmartPointer<vtkCaptionActor2D> _caption; // Shared between all
    vtkSmartPointer<vtkTextActor> _text;         // Shared between all
    bool hasView( const FV*) const;
    void purge( FV*) override;
};  // end class

}   // end namespace
}   // end namespace

#endif
