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

#ifndef FACE_TOOLS_FACE_VIEW_H
#define FACE_TOOLS_FACE_VIEW_H

#include "FaceModel.h"
#include "BoundaryView.h"
#include "SurfacePathDrawer.h"
#include "LandmarkGroupView.h"
#include "VisualisationAction.h"

namespace FaceTools
{

class ModelInteractor;  // For friend declaration


class FaceTools_EXPORT FaceView : public QObject
{ Q_OBJECT
public:
    FaceView( ModelViewer*, FaceModel*);
    virtual ~FaceView();

    FaceModel* getModel() const { return _fmodel;}
    bool isModelShown() const;
    bool isBoundaryShown() const;
    bool areLandmarksShown() const;

    // Can this model be visualised in the given manner?
    bool canVisualise( VisualisationAction*) const;

    // Returns true iff this model (or any of its landmarks)
    // is under the given viewport coordinates.
    bool isPointedAt( const QPoint&) const;

    // Set v to the projected 3D position of point p, returning
    // true iff p projects onto the model's surface.
    bool calcSurfacePosition( const QPoint& p, cv::Vec3f&) const;

    // Returns the name of any landmark under the given coordinates,
    // or an empty string if no landmarks belonging to this model
    // are under the coordinates.
    std::string isLandmarkPointedAt( const QPoint&) const;

    RFeatures::CameraParams getCamera() const;

    const vtkActor* getActor() const;

    const VisualisationAction* getCurrentVisualisation() const;

signals:
    void onChangedVisualisation( const VisualisationAction*);
    void onShowBoundary( bool);

public slots:
    // Set (and generate if necessary) current model visualisation and showModel(true).
    // Fires onChangedVisualisation immediately afterwards.
    void visualise( VisualisationAction*);
 
    // Shows or hides the model's current visualisation (also ensures proper viewer config).
    // Implies applyVisualisationOptions.
    void showModel( bool);

    void applyVisualisationOptions( const VisualisationOptions&); // Apply global visualisation options and update render.
    void showBoundary( bool); // Show the boundary (if available).
    void showLandmarks( bool); // Show the landmarks (if available).
    void showLandmark( const std::string&, bool); // Show a specific landmark (if available).
    void highlightLandmark( const std::string&, bool); // Temporarily highlight a landmark
    void orientCameraToFace(); // Set camera to view the model from the front at standard range.
    void setCameraToOrigin();  // Reset camera to world coordinate origin oriented along Z axis.
    bool saveSnapshot() const; // Saves a 2D snapshot

    // Project p to the point on the face, setting the camera focus there.
    // Returns true iff p projected onto the face.
    bool setCameraFocus( const QPoint& p);

private slots:
    void forceRevisualise();
    void showFaceDetection();
    void drawPath( const QPoint&);    // Forwards through to SurfacePathDrawer
    void finishPath( const QPoint&);  // Forwards through to SurfacePathDrawer
    friend class ModelInteractor;

private:
    ModelViewer* _viewer;
    FaceModel* _fmodel;
    boost::unordered_map<VisualisationAction*, vtkSmartPointer<vtkActor> > _allvis;
    VisualisationAction* _curvis;    // Currently active visualisation
    bool _inview;
    BoundaryView _bview;
    SurfacePathDrawer _pathDrawer;
    LandmarkGroupView _lview;
    VisualisationOptions _visopts;

    FaceView( const FaceView&);       // No copy
    void operator=( const FaceView&); // No copy
};  // end class

}   // end namespace

#endif
