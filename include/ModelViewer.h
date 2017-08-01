#ifndef FACE_TOOLS_MODEL_VIEWER_H
#define FACE_TOOLS_MODEL_VIEWER_H

/**
 * Simplified wrapper interface for QTools::VtkActorViewer.
 */

#include "FaceTools_Export.h"
#include <ObjModel.h>       // RFeatures
#include <CameraParams.h>   // RFeatures
#include <VtkActorViewer.h> // QTools
#include <MetricMapper.h>   // RVTK
#include <ScalarLegend.h>   // RVTK
#include <Axes.h>           // RVTK

namespace FaceTools
{

class FaceTools_EXPORT ModelViewer
{
public:
    ModelViewer( const cv::Size& viewerDims=cv::Size(512,512), bool useFloodLights=true, bool offscreenRendering=false);
    explicit ModelViewer( QTools::VtkActorViewer*);
    ~ModelViewer();

    void enableAxes( bool);
    void show();
    void hide();

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

    // Add custom surface actor metric mappings with metric value colour mapping range.
    int add( RVTK::MetricInterface*,
            float minv, const cv::Vec3b& minColourMapping,
            float maxv, const cv::Vec3b& maxColourMapping,
            int ncolours=100,   // Number of discrete colours to use
            bool showLegend=true);

    bool remove( int id);
    void removeAll();   // Remove all models

    cv::Point2f projectProp( const cv::Vec3f&) const;   // Project to viewport proportion.
    cv::Point project( const cv::Vec3f&) const;     // Project to pixel coords.
    cv::Vec3f project( const cv::Point2f&) const;   // Project to world coords.
    cv::Vec3f project( const cv::Point&) const;     // Project to world coords.

    void resetDefaultCamera( float camRng=650.0f);    // Default range is 65 cm from origin directly along +Z.
    void fitCamera( double radius); // Adjust FoV to fit an object of given radius at current distance.
    void setCamera( const RFeatures::CameraParams&);
    // Set the camera using vectors (normal is towards the camera).
    void setCamera( const cv::Vec3f& focus, const cv::Vec3f& normal, const cv::Vec3f& upvector, float camRng=650.0f);
    RFeatures::CameraParams getCamera() const;

    cv::Size_<int> getViewportSize() const; // Pixel width and height of viewport (image plane).

    QTools::VtkActorViewer& raw() { return *_viewer;}

    // Updates the render and shows what's currently in the display as an OpenCV snapshot.
    void showSnapshot( bool waitForInput=false);

private:
    QTools::VtkActorViewer* _viewer;
    RVTK::ScalarLegend* _scalarLegend;
    RVTK::Axes* _axes;
    bool _dodel;
    int _addedModelID;
    boost::unordered_map<int, std::vector<vtkSmartPointer<vtkActor> > > _actors;
    int addPointsActor( vtkSmartPointer<vtkActor>, const VisOptions&);
    void init();
    ModelViewer( const ModelViewer&);   // NO COPY
    void operator=( const ModelViewer&);// NO COPY
};  // end class

}   // end namespace

#endif

