/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ModelViewerInteractor.h>
#include <VtkActorViewer.h> // QTools
#include <ScalarLegend.h>   // RVTK
#include <Axes.h>           // RVTK
//#include <ObjModel.h>       // RFeatures
#include <CameraParams.h>   // RFeatures
#include <QColor>
#include <unordered_map>
#include <unordered_set>

namespace FaceTools {

class FaceTools_EXPORT ModelViewer : public QWidget
{ Q_OBJECT
public:
    ModelViewer( QWidget* parent=NULL, bool useFloodLights=false);
    virtual ~ModelViewer();

    // Interactor::ModelViewerInteractor (MVI) calls the protected attach and detach functions. Multiple different
    // MVI instances (even of the same type) can be attached at once to this viewer, but MVI instances can only
    // belong to a single ModelViewer at a time.
    bool isAttached( Interactor::MVI*) const;  // Returns whether the given MVI is attached.

    // Transfer to parameter viewer all MVIs attached to this viewer (returns # moved).
    // If the parameter viewer is the same as this one, nothing occurs and zero is returned.
    size_t transferInteractors( ModelViewer*);

    // Set/get interaction mode.
    void setInteractionMode( QTools::InteractionMode m) { _qviewer->setInteractionMode(m);}
    QTools::InteractionMode interactionMode() const { return _qviewer->interactionMode();}

    // Lock/unlock camera/actor interaction.
    int lockInteraction() { return _qviewer->lockInteraction();}
    bool unlockInteraction( int lkey) { return _qviewer->unlockInteraction(lkey);}
    bool isInteractionLocked() const { return _qviewer->isInteractionLocked();}

    const QPoint& mouseCoords() const { return _qviewer->mouseCoords();}
    bool mouseOnRenderer() const { return _qviewer->mouseOnRenderer();}
    QPoint mapToGlobal( const QPoint& p) const { return _qviewer->mapToGlobal(p);}

    void setSize( const cv::Size&);

    void addToLayout( QLayout*);
    void removeFromLayout( QLayout*);

    void show();
    void hide();

    void showLegend( bool);
    bool legendShown() const;
    void showAxes( bool);
    bool axesShown() const;

    void enableFloodLights( bool);  // Set true for textured objects, false for surface.
    bool floodLightsEnabled() const;

    enum Visualisation
    {
        TEXTURE_VISUALISATION = 1,
        SURFACE_VISUALISATION,
        POINTS_VISUALISATION,
        WIREFRAME_VISUALISATION
    };  // end enum

    struct VisOptions
    {
        VisOptions( Visualisation v=TEXTURE_VISUALISATION, float rc=1.0f, float gc=1.0f, float bc=1.0f, float ac=1.0f,
                                                bool bf=false, float ps=1.0f, float lw=1.0f)
            : vis(v), r(rc),g(gc),b(bc),a(ac), backfaceCulling(bf), pointSize(ps), lineWidth(lw) {}

        VisOptions( float rc, float gc, float bc, float ac=1.0f, bool bf=false, float ps=1.0f, float lw=1.0f)
            : vis(SURFACE_VISUALISATION), r(rc),g(gc),b(bc),a(ac), backfaceCulling(bf), pointSize(ps), lineWidth(lw) {}

        VisOptions( Visualisation v, bool bf, float ps=1.0f, float lw=1.0f)
            : vis(v), r(1.0f), g(1.0f), b(1.0f), a(1.0f), backfaceCulling(bf), pointSize(ps), lineWidth(lw) {}

        Visualisation vis;
        float r, g, b, a;
        bool backfaceCulling;
        float pointSize;
        float lineWidth;
    };  // end struct

    void updateRender();    // Call after making changes to the view content

    // Add or remove a prop.
    void add( vtkProp*);
    void remove( vtkProp*);

    // Set the legend title and colours lookup table for the scalar legend.
    void setLegend( const std::string& title, vtkLookupTable*);

    cv::Point2f projectProp( const cv::Vec3f&) const;   // Project to viewport proportion.
    cv::Point project( const cv::Vec3f&) const;         // Project to pixel coords.
    cv::Vec3f project( const cv::Point2f&) const;       // Project to world coords.
    cv::Vec3f project( const cv::Point&) const;         // Project to world coords.
    cv::Vec3f project( const QPoint&) const;            // Project to world coords.

    // Return the prop under the given coords or NULL if none pointed at.
    const vtkProp* getPointedAt( const cv::Point2f&) const;
    const vtkProp* getPointedAt( const cv::Point&) const;
    const vtkProp* getPointedAt( const QPoint&) const;

    // Returns true iff given coords pick out the given actor.
    bool getPointedAt( const QPoint&, const vtkActor*) const;

    // Project the given point to a world position on the given prop, returning true.
    // Return false if the point doesn't project onto the given prop. Out param wpos
    // is set only if the function returns true.
    bool calcSurfacePosition( const vtkProp*, const cv::Point2f&, cv::Vec3f& wpos) const;
    bool calcSurfacePosition( const vtkProp*, const cv::Point&, cv::Vec3f& worldPos) const;
    bool calcSurfacePosition( const vtkProp*, const QPoint&, cv::Vec3f& worldPos) const;

    void resetDefaultCamera( float camRng=650.0f);    // Default range is 65 cm from origin directly along +Z.
    void fitCamera( double radius); // Adjust FoV to fit an object of given radius AT CURRENT DISTANCE
    void setCamera( const RFeatures::CameraParams&);
    // Set the camera using vectors (normal is towards the camera).
    void setCamera( const cv::Vec3f& focus, const cv::Vec3f& normal, const cv::Vec3f& upvector, float camRng=650.0f);
    void setFocus( const cv::Vec3f&);
    RFeatures::CameraParams getCamera() const;
    float cameraDistance() const;   // Distance between position and focus

    size_t getWidth() const;    // Return the width of the viewport in pixels
    size_t getHeight() const;   // Return the height of the viewport in pixels

    vtkRenderer* getRenderer() { return _qviewer->getRenderer();}
    const vtkRenderer* getRenderer() const { return _qviewer->getRenderer();}
    const vtkRenderWindow* getRenderWindow() const { return _qviewer->getRenderWindow();}

    void setCursor( QCursor);

    cv::Mat_<cv::Vec3b> grabImage() const;  // Retrieves what's currently being rendered as an OpenCV image matrix.
    bool saveSnapshot() const;  // User save of grabImage to file.

protected:
    bool attach( Interactor::MVI*);  // Attach interactor returning false iff already attached.
    bool detach( Interactor::MVI*);  // Detach interactor returning false iff already detached.
    friend class Interactor::ModelViewerInteractor;    // Calls attach and detach passing in self as parameter.

private:
    QTools::VtkActorViewer *_qviewer;
    RVTK::ScalarLegend *_scalarLegend;
    RVTK::Axes *_axes;
    bool _floodLightsEnabled;
    std::unordered_set<Interactor::MVI*> _interactors;
    ModelViewer( const ModelViewer&) = delete;
    void operator=( const ModelViewer&) = delete;
};  // end class

}   // end namespace

#endif
