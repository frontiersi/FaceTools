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

#ifndef FACE_TOOLS_MODEL_VIEWER_H
#define FACE_TOOLS_MODEL_VIEWER_H

#include "Interactor/ViewerNotifier.h"
#include "Interactor/MouseHandler.h"
#include <QTools/VtkActorViewer.h>
#include <r3d/CameraParams.h>
#include <QColor>
#include <unordered_map>
#include <unordered_set>

namespace FaceTools {

class FaceTools_EXPORT ModelViewer : public QWidget
{ Q_OBJECT
public:
    ModelViewer( QWidget* parent=nullptr, bool useFloodLights=false);
    ~ModelViewer() override;

    void setCameraInteraction() { _qviewer->setCameraInteraction();}
    void setActorInteraction( const vtkProp3D *p) { _qviewer->setActorInteraction(p);}

    QTools::InteractionMode interactionMode() const { return _qviewer->interactionMode();}

    // Lock/unlock camera/actor interaction.
    int lockInteraction() { return _qviewer->lockInteraction();}
    bool unlockInteraction( int lkey) { return _qviewer->unlockInteraction(lkey);}
    bool isInteractionLocked() const { return _qviewer->isInteractionLocked();}

    // Get the mouse coordinates relative to the underlying renderer.
    QPoint mouseCoords() const { return _qviewer->mapFromGlobal( QCursor::pos());}

    void setSize( const cv::Size&);

    void show();
    void hide();

    void enableFloodLights( bool);  // Set true for textured objects, false for surface.
    bool floodLightsEnabled() const;

    void setBackgroundColour( const QColor&);
    QColor backgroundColour() const;

    // Add or remove a prop.
    void add( vtkProp*);
    void remove( vtkProp*);
    void clear();   // Clear all props

    Vec3f project( const QPoint&) const;    // Project to world coords (not thread safe)
    QPoint project( const Vec3f&) const;    // Project to display coords (not thread safe)

    // Return the prop under the given coords or null if none pointed at.
    const vtkProp* getPointedAt( const cv::Point2f&) const;
    const vtkProp* getPointedAt( const cv::Point&) const;
    const vtkProp* getPointedAt( const QPoint&) const;

    // Returns true iff given coords pick out the given actor.
    bool getPointedAt( const QPoint&, const vtkActor*) const;

    // Project the given point to a world position on the given prop, returning true.
    // Return false if the point doesn't project onto the given prop. Out param wpos
    // is set only if the function returns true.
    // *** THESE FUNCTIONS ARE NOT THREAD SAFE! Use in the GUI thread. ***
    bool calcSurfacePosition( const vtkProp*, const cv::Point2f&, Vec3f& wpos) const;
    bool calcSurfacePosition( const vtkProp*, const cv::Point&, Vec3f& worldPos) const;
    bool calcSurfacePosition( const vtkProp*, const QPoint&, Vec3f& worldPos) const;

    void resetDefaultCamera( float camRng=650.0f);    // Default range is 65 cm from origin directly along +Z.
    void fitCamera( float radius); // Adjust FoV to fit an object of given radius AT CURRENT DISTANCE

    void setCamera( const r3d::CameraParams&);
    void setCamera( const Vec3f& focus, const Vec3f& position);
    r3d::CameraParams camera() const;

    void setCameraFocus( const Vec3f&);
    Vec3f cameraFocus() const;

    void setCameraPosition( const Vec3f&);
    Vec3f cameraPosition() const;

    // Calculate and return "snapping" distance as a proportion p of the
    // distance between the camera's position and focus.
    float snapRange( float p=0.015f) const;

    void refreshClippingPlanes();

    size_t getWidth() const;    // Return the width of the viewport in pixels
    size_t getHeight() const;   // Return the height of the viewport in pixels

    vtkRenderer* getRenderer() { return _qviewer->getRenderer();}
    const vtkRenderer* getRenderer() const { return _qviewer->getRenderer();}
    vtkRenderWindow* getRenderWindow() { return _qviewer->getRenderWindow();}
    const vtkRenderWindow* getRenderWindow() const { return _qviewer->getRenderWindow();}

    void setCursor( QCursor);

    cv::Mat_<cv::Vec3b> grabImage() const;  // Retrieves what's currently being rendered as an OpenCV image matrix.
    bool saveSnapshot() const;  // User save of grabImage to file.

    void setParallelProjection( bool);
    bool parallelProjection() const;

    void updateRender();    // Call after making changes to content

protected:
    // Attach/detach interactors and mouse handlers returning false iff already attached.
    bool attach( Interactor::ViewerNotifier*);
    bool detach( Interactor::ViewerNotifier*);
    bool attach( Interactor::MouseHandler*);
    bool detach( Interactor::MouseHandler*);
    friend class Interactor::ViewerNotifier;    // Calls attach and detach passing in self as parameter.
    friend class Interactor::MouseHandler;

private:
    QTools::VtkActorViewer *_qviewer;
    bool _floodLightsEnabled;
    ModelViewer( const ModelViewer&) = delete;
    void operator=( const ModelViewer&) = delete;
};  // end class

}   // end namespace

#endif
