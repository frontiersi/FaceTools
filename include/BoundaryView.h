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

#ifndef FACE_TOOLS_BOUNDARY_VIEW_H
#define FACE_TOOLS_BOUNDARY_VIEW_H

#include "ModelViewer.h"
#include "ObjMetaData.h"
#include "VisualisationOptions.h"
#include <ModelPathDrawer.h>    // RVTK

namespace FaceTools
{

// Translate VTK callbacks into Qt signals so clients know when the boundary has changed.
// Create this here instead of making it an internal class and placing signal in BoundaryView
// (making it a Q_OBJECT) because that signal would need to be routed back through the
// encapsulating objects via signal/slot connections. Cleaner for the controlling class to listen
// for the update event and request the updated  boundary data from the BoundaryView instance.
class FaceTools_EXPORT BoundaryViewEventObserver : public QObject, public RVTK::ModelPathEventObserver
{ Q_OBJECT
public:
    virtual void interactionEvent( const RVTK::ModelPathDrawer*);
signals:
    void updatedBoundary( const std::vector<cv::Vec3f>&); // Emitted with new boundary when updated
};  // end class


class FaceTools_EXPORT BoundaryView
{
public:
    BoundaryView( ModelViewer*, const ObjMetaData::Ptr, BoundaryViewEventObserver*);

    void allowAdjustment( bool); // Respond to user interaction or not
    bool canAdjust() const;
    void show( bool enable);
    bool isShown() const;
    void reset( const vtkActor*); // Reset from the ObjMetaData
    void setVisualisationOptions( const VisualisationOptions::Boundary&);

private:
    const ObjMetaData::Ptr _objmeta;
    RVTK::ModelPathDrawer::Ptr _boundary;
    VisualisationOptions::Boundary _visopts;

    BoundaryView( const BoundaryView&);   // No copy
    void operator=( const BoundaryView&); // No copy
};  // end class

}   // end namespace

#endif
