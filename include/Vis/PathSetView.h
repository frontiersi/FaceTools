/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_PATH_SET_VIEW_H
#define FACE_TOOLS_PATH_SET_VIEW_H

/**
 * Used by PathSetVisualisation to manage the view elements of paths
 * by path ID for a specific pathset (associated with a FaceModel).
 * Also provides for hash table lookup access of paths by vtkProp.
 */

#include "PathView.h"
#include <PathSet.h>
#include <vtkTextActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathSetView
{
public:
    explicit PathSetView( const PathSet&);
    virtual ~PathSetView();

    bool isVisible() const;                             // Returns true iff ANY path shown
    void setVisible( bool, ModelViewer*);               // Show/hide all paths (also sets current viewer)

    void setText( int pathID, int xpos, int ypos);
    void setTextVisible( bool);
    void showPath( bool, int pathID);                   // Show/hide individual path
    bool isPathVisible( int pathID) const;              // Returns true iff a particular path is shown
    const std::unordered_set<int>& visible() const;     // Returns the set of visible paths

    PathView::Handle* addPath( int pathId);             // Add path from dataset returning a handle (both in same position)
    PathView::Handle* handle( const vtkProp*) const;    // Return the handle mapped to the given prop (if any).
    PathView* pathView( int pathId) const;              // Return the requested path view.

    // Update the given path from the PathSet (remove if necessary).
    void updatePath( int pathId, const cv::Matx44d& d=cv::Matx44d::eye());

    // Refresh all path visualisations to match _paths.
    void refresh( const cv::Matx44d& d=cv::Matx44d::eye());

private:
    const PathSet& _paths;
    ModelViewer *_viewer;
    std::unordered_map<int, PathView*> _views;
    std::unordered_map<const vtkProp*, PathView::Handle*> _handles;
    std::unordered_set<int> _visible;   // Which paths are visible
    vtkNew<vtkTextActor> _text; // Bottom right text

    PathSetView( const PathSetView&) = delete;
    void operator=( const PathSetView&) = delete;
};  // end class

}}   // end namespaces

#endif
