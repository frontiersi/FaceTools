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

#ifndef FACE_TOOLS_FACE_MODEL_INTERACTOR_H
#define FACE_TOOLS_FACE_MODEL_INTERACTOR_H

#include "InteractiveModelViewer.h"
#include "InteractionInterface.h"
#include "FaceView.h"

namespace FaceTools
{

class FaceTools_EXPORT ModelInteractor : public InteractionInterface
{ Q_OBJECT
public:
    // Viewer not const due to setCursor calls.
    ModelInteractor( InteractiveModelViewer*, FaceModel*, const QList<QAction*>*);
    virtual ~ModelInteractor();

    const FaceView* getView() const { return _fview;}
    FaceView* getView() { return _fview;}
    const FaceModel* getModel() const { return _fmodel;}
    FaceModel* getModel() { return _fmodel;}

    bool isDrawingPath() const { return _isDrawingPath;}        // Currently drawing path
    bool isMovingLandmark() const { return _isMovingLandmark;}  // Currently moving landmark
    const QPoint& getMouseCoords() const { return _viewer->getMouseCoords();}   // Last mouse coords
    bool calcSurfacePosition( cv::Vec3f&) const;     // Convenience for getView()->calcSurfacePosition( getMouseCoords(), cv::Vec3f&)

    // Cause enableActionable to be fired (see signal below). Safe to call
    // multiple times over with the same value; enableActionable will only
    // fire when the actionable state of this object changes value.
    void setInteractive( bool);
    bool isInteractive() const { return _interactive;}

signals:
    // Entering/exiting the 2D bounds of the model.
    void onEnteringModel( const QPoint&);
    void onExitingModel( const QPoint&);

    // Entering/exiting a landmark (don't care about the 2D position).
    void onEnteringLandmark( const std::string&, const QPoint&);
    void onExitingLandmark( const std::string&, const QPoint&);

protected:
    virtual void middleDrag( const QPoint&);
    virtual void middleButtonUp( const QPoint&);
    virtual void leftButtonDown( const QPoint&);
    virtual void leftButtonUp( const QPoint&);
    virtual void leftDoubleClick( const QPoint&);
    virtual void leftDrag( const QPoint&);
    virtual void mouseMove( const QPoint&);

private:
    InteractiveModelViewer* _viewer;
    FaceModel* _fmodel;
    FaceView* _fview;
    const QList<QAction*> *_actions;

    bool _isDrawingPath;
    bool _isMovingLandmark;
    bool _modelHoverOld;
    std::string _lmHoverOld;
    std::string _pickedLandmark;
    bool _interactive;

    void doMouseHover( const QPoint&);
    ModelInteractor( const ModelInteractor&); // No copy
    void operator=( const ModelInteractor&);  // No copy
};  // end class

}   // end namespace

#endif
