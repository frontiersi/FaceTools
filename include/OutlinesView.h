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

#ifndef FACE_TOOLS_OUTLINES_VIEW_H
#define FACE_TOOLS_OUTLINES_VIEW_H

#include "ModelViewer.h"

namespace FaceTools
{

class FaceTools_EXPORT OutlinesView
{
public:
    // Outlines are extracted on creation. If model changes, a new OutlinesView will need to be created.
    OutlinesView( const RFeatures::ObjModel::Ptr, const ModelViewer::VisOptions* vo=NULL);
    ~OutlinesView();

    void reset( const RFeatures::ObjModel::Ptr);

    // Call updateRender on the viewer after setVisible.
    void setVisible( bool, ModelViewer* viewer);
    bool isVisible() const;

private:
    ModelViewer::VisOptions _voptions;
    std::list<std::vector<cv::Vec3f> > _vloops;
    IntSet _bloops;
    ModelViewer *_viewer;
};  // end class

}   // end namespace

#endif
