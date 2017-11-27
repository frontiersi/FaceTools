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

#include "InteractionInterface.h"
#include "ModelOptions.h"
#include "FaceModel.h"
#include "FaceView.h"
#include "Landmarks.h"

namespace FaceTools
{

class LegendRange;
class BoundaryView;
class OutlinesView;
class LandmarkGroupView;
class VisualisationAction;
class InteractiveModelViewer;

class FaceTools_EXPORT FaceControl : public InteractionInterface
{ Q_OBJECT
public:
    explicit FaceControl( FaceModel*);
    ~FaceControl() override;

    void setViewer( InteractiveModelViewer*);   // Originally NULL
    InteractiveModelViewer* getViewer() const { return _viewer;}

    // Add/remove references to controller actions not provided in the constructor.
    void addController( QAction*);
    void removeController( QAction*);

    // Cause connected actions to treat this model as currently being under
    // control of the interface (default), or tell connected actions that
    // this model is not currently under control (controlled=false).
    // NB even if controlled=false, actions may still choose to act over
    // their connected models if the nature of the action does not require
    // the model to be under direct user control.
    void setControlled( bool controlled=true);
    bool isControlled() const;

    void setVisualisation( VisualisationAction*);
    VisualisationAction* getVisualisation() const { return _curvis;}

    void setOptions( const ModelOptions&);
    const ModelOptions& getOptions() const { return _opts;}

    void showSelected( bool);    // viewUpdated NOT emitted for visualising the outline!
    bool isSelected() const;

    void showBoundary( bool);
    bool isBoundaryShown() const;
    void showLandmarks( bool);
    bool areLandmarksShown() const; // Returns true iff at least one landmark visible
    void showLandmark( bool, const std::string&);
    void highlightLandmark( bool, const std::string&);

    // Update landmarks. If making many updates in short succession, call with updateModel=false
    // until the very last update so that the FaceModel does not signal listening clients more
    // than necessary (clients may run long operations on response to a FaceModel metadata update).
    void updateLandmark( const std::string&, const cv::Vec3f*, bool updateModel=true);
    void updateLandmark( const Landmarks::Landmark&);   // Updates view and model
    void updateMesh( const RFeatures::ObjModel::Ptr);   // Also calls resetVisualisation
    FaceModel* getModel() const { return _fmodel;}
    FaceView* getView() const { return _fview;}
    LegendRange* getLegend() const { return _legend;}

    bool belongs( const vtkProp*) const; // Returns true iff this model or one of its components is the given prop.
    bool isPointedAt() const;   // Returns true iff model or any of its components are under mouse cursor.

    // Add or remove props that are considered part of this model for mouse events.
    void addTempMemberProp( const vtkProp*);
    void removeTempMemberProp( const vtkProp*);

    // Returns true iff one of the props added using addTempMemberProp matches
    // the provided prop. If the provided prop is NULL, the prop currently being
    // pointed to by the mouse is checked.
    bool isTempPropPointedAt( const vtkProp* prop=NULL) const;

    // Returns the name of any landmark under the given coordinates (or the current mouse coords if NULL).
    // An empty string is returned if no landmarks owned by this model are under the given coordinates.
    std::string isLandmarkPointedAt( const QPoint* p=NULL) const;

    void resetVisualisation();    // Refresh from model (useful if multiple FaceControls on a FaceModel).

signals:
    void meshUpdated(); // Emitted when model geometry on underlying FaceModel changed.
    void metaUpdated(); // Emitted when meta-data on underlying FaceModel changed.
    void viewUpdated(); // Emitted when various view elements are added/removed EXCEPT viewing the outline.

    void onEnteringModel( const QPoint&);
    void onExitingModel( const QPoint&);
    void onEnteringLandmark( const std::string&, const QPoint&);
    void onExitingLandmark( const std::string&, const QPoint&);

    // Emiited whether or not point is on the model's surface.
    void onLeftButtonUp( const QPoint&);
    // Emitted only if mouse on model or its elements.
    void onLeftButtonDown( const QPoint&);
    void onLeftDoubleClick( const QPoint&);
    void onLeftDrag( const QPoint&);
    void onMouseMove( const QPoint&);

protected:
    void leftButtonUp( const QPoint&) override;
    void leftButtonDown( const QPoint&) override;
    void leftDoubleClick( const QPoint&) override;
    void leftDrag( const QPoint&) override;
    void mouseMove( const QPoint&) override;

private slots:
    void doMeshUpdated();

private:
    InteractiveModelViewer *_viewer;
    FaceModel* _fmodel;
    FaceView* _fview;
    OutlinesView* _oview;
    BoundaryView* _bview;
    LandmarkGroupView* _lview;
    LegendRange* _legend;
    VisualisationAction* _curvis;
    ModelOptions _opts;

    bool _modelHoverOld;
    std::string _lmHoverOld;
    bool _controlled;
    boost::unordered_set<QAction*> _actions;
    boost::unordered_set<const vtkProp*> _tmpProps;

    FaceControl( const FaceControl&);     // No copy
    void operator=( const FaceControl&);  // No copy
};  // end class

}   // end namespace
    
#endif
