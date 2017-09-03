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

#ifndef FACE_TOOLS_FACE_MODEL_WIDGET_H
#define FACE_TOOLS_FACE_MODEL_WIDGET_H

#include "FaceModelInteractor.h"
#include "ContextMenuHandler.h"
#include "SurfacePathDrawer.h"

#include <QActionGroup>

namespace Ui { class FaceModelWidget;}

namespace FaceTools
{

class FaceTools_EXPORT FaceModelWidget : public QWidget
{ Q_OBJECT
public:
    FaceModelWidget( const QActionGroup& viewGroup, QWidget *parent = 0);
    virtual ~FaceModelWidget();

    void highlightFrame( bool);

    void showMessageBottomLeft( const std::string& msg="");  // Left justified
    void showMessageBottomRight( const std::string& msg=""); // Right justified

    void getCamera( CameraParams*) const;
    void setCameraOrientationFromModel( int modelID);

    // Returns the ID of the model currently selected by the user. Returns 0 if no model loaded.
    int getCurrentlySelectedComboBoxModelID() const { return _currentlySelectedModelID;}

    // Gets the world points of all of the vertices on the current facial boundary.
    // Returns number of vertices.
    size_t getAllBoundaryVertices( std::vector<cv::Vec3f>& bverts) const;

    size_t getBoundaryHandles( std::vector<cv::Vec3f>& bhandles) const;

    // Project wv into the view and set the projected point with coordinates defined by the
    // scale of the projection image plane (given by imageDims). If imageDims is empty (default),
    // the size of the image plane is set as the current dimensions of the ModelViewer.
    cv::Point projectToPlane( const cv::Vec3f& wv, cv::Size imageDims=cv::Size()) const;

    // Return the updated image grabs. Keep landmarks/boundary set to false to ensure that these
    // aspects don't appear in the images.
    const FaceApp::Visualisation::ImageGrabber& getUpdatedImages( bool landmarks=false, bool boundary=false);

    // Calculates the position on the surface of the specified model given a top-left origin 2D
    // reference point (as a proportion of the view window). Records the position in worldPos.
    // False is returned iff no position on the specified model is found. Other models are ignored.
    bool calcSurfacePosition( int modelID, const cv::Point2f&, cv::Vec3f& worldPos) const;

    void setCurrentModel( int modelID);
    void refreshCurrentModel();

signals:
    void onReceivedMousePressEvent( FaceApp::GUI::FaceModelWidget*);
    void onModelSelected( int); // Emitted when user selects model from dropdown and view has changed.
    void onDeleteMarker( int);
    void onUpdateMarkerPosition( int, const cv::Vec3f&);
    void onBoundaryHandlesChanged();

public slots:
    void doOnAddModel( int);     // Adds model to dropdown, updates view, and emits onModelSelected
    void doOnRemoveModel( int);  // Removes model from dropdown and from viewer if present and emits onModelSelected
    void doOnSetCameraFrontProfile(); // Current model
    void doOnSetCameraLeftProfile();  // Current model
    void doOnSetCameraRightProfile(); // Current model
    void doOnPrintCameraDetails();
    void doOnApplyVisualisation( QAction*);   // Set to NULL for default visualisation
    void doOnShowBoundary( bool);
    void doOnShowLandmarks( bool);

protected:
    virtual void mousePressEvent( QMouseEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

private slots:
    void doOnMousePressEvent();
    void doOnTakeSnapshot();
    void doOnModelSelectorCurrentIndexChanged(int);
    void doOnShowContextMenu( const QPoint&);
    void doOnDeleteMarker();
    void doOnChangeCursor( QCursor);
    void doOnStartedMovingLandmark(int);
    void doOnMovingLandmark(int);
    void doOnFinishedMovingLandmark(int);
    void doOnDoingLandmarkHover(int);
    void doOnFinishedLandmarkHover(int);
    void doOnShowLandmark( int, bool);
    void doOnSetFocus();

private:
    Ui::FaceModelWidget *ui;

    FaceModelInteractor *_interactor;
    ContextMenuHandler *_contextMenuHandler;
    SurfacePathDrawer *_pathDrawer;

    QActionGroup _visualisationGroup;
    IntSet _modelIDs; // The models available to this viewer

    void removeModelFromViewer( int modelID);
    int getModelIDFromComboBoxRow( int rowIdx) const;
    int getComboBoxRowFromModelID( int modelID) const;
    void addActionsToVisualisationToolBar( const QActionGroup&);
    void addDisplayedMarkers( int modelID);
    void removeDisplayedMarkers( int modelID);
    void addModel( int modelID);
    void updateUI();
    void showVisualisationTitle( const QString&);

    FaceModelWidget( const FaceModelWidget&); // No copy
    void operator=( const FaceModelWidget&);   // No copy
};  // end class

}   // end namespace

#endif

