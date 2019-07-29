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

    void setText( const Path&, int xpos, int ypos);
    void setTextVisible( bool);

    void addPath( const Path&);
    void movePath( const Path&);
    void erasePath( int pathId);

    PathView::Handle* handle( const vtkProp*) const;    // Return the handle mapped to the given prop (if any).
    PathView* pathView( int pathId) const;              // Return the requested path view.

    // Refresh all path visualisations to match _paths.
    void sync( const PathSet&, const cv::Matx44d&);

    void updateTextColours();

private:
    ModelViewer *_viewer;
    std::unordered_map<int, PathView*> _views;
    std::unordered_map<const vtkProp*, PathView::Handle*> _handles;
    std::unordered_set<int> _visible;   // Which paths are visible
    vtkNew<vtkTextActor> _text; // Bottom right text

    void _showPath( bool, int pathID);                   // Show/hide individual path
    PathSetView( const PathSetView&) = delete;
    void operator=( const PathSetView&) = delete;
};  // end class

}}   // end namespaces

#endif
