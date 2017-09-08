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

#ifndef FACE_TOOLS_MODEL_VIEWER_H
#define FACE_TOOLS_MODEL_VIEWER_H

/**
 * Simplified wrapper interface for QTools::VtkActorViewer.
 */

#include "FaceTools_Export.h"
#include <ObjModel.h>       // RFeatures
#include <CameraParams.h>   // RFeatures
#include <VtkActorViewer.h> // QTools
#include <SurfaceMapper.h>  // RVTK
#include <ScalarLegend.h>   // RVTK
#include <Axes.h>           // RVTK
#include <QColor>

namespace FaceTools
{

class FaceTools_EXPORT ModelViewer
{
public:
    explicit ModelViewer( QTools::VtkActorViewer*);
    ModelViewer( bool useFloodLights=true, bool offscreenRendering=false);
    virtual ~ModelViewer();

    void show();
    void hide();

    void showLegend( bool);
    void showAxes( bool);

    void enableFloodLights( bool);  // Set true for textured objects, false for surface.

    enum Visualisation
    {
        VisTexture = 1,
        VisSurface,
        VisPoints,
        VisWireframe
    };  // end enum

    struct VisOptions
    {
        VisOptions( Visualisation v=VisTexture, float rc=1.0f, float gc=1.0f, float bc=1.0f, float ac=1.0f,
                                                bool bf=false, float ps=1.0f, float lw=1.0f)
            : vis(v), r(rc),g(gc),b(bc),a(ac), backfaceCulling(bf), pointSize(ps), lineWidth(lw) {}

        VisOptions( Visualisation v, bool bf, float ps=1.0f, float lw=1.0f)
            : vis(v), r(1.0f), g(1.0f), b(1.0f), a(1.0f), backfaceCulling(bf), pointSize(ps), lineWidth(lw) {}

        Visualisation vis;
        float r, g, b, a;
        bool backfaceCulling;
        float pointSize;
        float lineWidth;
    };  // end struct

    void updateRender();    // Call after making changes to the view content

    // Add a points actor in various formats.
    int addPoints( const std::vector<cv::Vec3f>& points, const VisOptions&);
    int addPoints( const RFeatures::ObjModel::Ptr, const VisOptions&);
    int addPoints( const RFeatures::ObjModel::Ptr, const IntSet& vset, const VisOptions&);

    // Add a line actor (make a loop if joinEnds=true).
    int addLine( const std::vector<cv::Vec3f>&, bool joinEnds, const VisOptions&);
    
    // Add line pairs: lp.size() must be even.
    int addLinePairs( const std::vector<cv::Vec3f>& lp, const VisOptions&);

    int add( const RFeatures::ObjModel::Ptr, const VisOptions& vo=VisOptions());

    // Add custom surface actor. Set metric value colour mapping with setLegendColours.
    int add( RVTK::SurfaceMapper*, float minv, float maxv);
    int add( vtkSmartPointer<vtkActor>, const std::string& legendTitle, float minv, float maxv);

    // Number of discrete colours to use
    void setLegendColours( const cv::Vec3b& minColourMapping, const cv::Vec3b& maxColourMapping, int ncolours=100);
    void setLegendColours( const QColor& minColourMapping, const QColor& maxColourMapping, int ncolours=100);

    bool remove( int id);   // Remove a prop using an ID returned from a successful add.
    void removeAll();       // Remove all props that were added using one of the add functions that returns an ID.

    // Add or remove an arbitrary prop - these props are NOT removed upon removeAll() and
    // must be removed manually (external references to these props must be retained).
    void add( const vtkProp*);
    void remove( const vtkProp*);

    cv::Point2f projectProp( const cv::Vec3f&) const;   // Project to viewport proportion.
    cv::Point project( const cv::Vec3f&) const;         // Project to pixel coords.
    cv::Vec3f project( const cv::Point2f&) const;       // Project to world coords.
    cv::Vec3f project( const cv::Point&) const;         // Project to world coords.

    // Return the prop under the given coords or NULL.
    const vtkProp* getPointedAt( const cv::Point2f&) const;
    const vtkProp* getPointedAt( const cv::Point&) const;
    const vtkProp* getPointedAt( const QPoint&) const;

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
    RFeatures::CameraParams getCamera() const;

    size_t getWidth() const;    // Return the width of the viewport in pixels
    size_t getHeight() const;   // Return the height of the viewport in pixels

    const vtkSmartPointer<vtkRenderer> getRenderer() const { return _qviewer->getRenderer();}
    const vtkSmartPointer<vtkRenderWindow> getRenderWindow() const { return _qviewer->getRenderWindow();}

    void setCursor( QCursor);

    // Updates the render and shows what's currently in the display as an OpenCV snapshot.
    void showSnapshot( bool waitForInput=false);

    // Allows user to save snapshot to file.
    bool saveSnapshot() const;

protected:
    QTools::VtkActorViewer* _qviewer;

private:
    RVTK::ScalarLegend* _scalarLegend;
    RVTK::Axes* _axes;
    bool _dodel;
    int _addedModelID;
    boost::unordered_map<int, std::vector<vtkProp*> > _props;
    int addPointsActor( vtkSmartPointer<vtkActor>, const VisOptions&);
    void init();
    ModelViewer( const ModelViewer&);       // NO COPY
    void operator=( const ModelViewer&);    // NO COPY
};  // end class

}   // end namespace

#endif

