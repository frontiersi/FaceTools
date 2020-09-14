/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include "PathView.h"
#include <FaceTools/PathSet.h>
#include <vtkTextActor.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT PathSetView
{
public:
    PathSetView();
    virtual ~PathSetView();

    void setVisible( bool, ModelViewer*);               // Show/hide all paths (also sets current viewer)
    bool isVisible() const;

    void setCaption( const Path&, int xpos, int ypos, const Mat3f&);
    void setCaptionVisible( bool);

    void sync( const PathSet&);
    void addPath( const Path&);
    void updatePath( const Path&);

    void erasePath( int pid);

    PathView::Handle* handle( const vtkProp*) const;    // Return the handle mapped to the given prop (if any).
    PathView* pathView( int pathId) const;              // Return the requested path view.

    void pokeTransform( const vtkMatrix4x4*);

    void updateTextColours();

private:
    ModelViewer *_viewer;
    std::unordered_map<int, PathView*> _views;
    std::unordered_map<const vtkProp*, PathView::Handle*> _handles;
    std::unordered_set<int> _visible;   // Which paths are visible
    vtkNew<vtkTextActor> _caption; // Bottom right text

    void _showPath( bool, int pathID);                   // Show/hide individual path
    PathSetView( const PathSetView&) = delete;
    void operator=( const PathSetView&) = delete;
};  // end class

}}   // end namespaces

#endif
