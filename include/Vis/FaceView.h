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

#ifndef FACE_TOOLS_FACE_VIEW_H
#define FACE_TOOLS_FACE_VIEW_H

/**
 * The container for coordinating the generation and application of visualisations for a
 * RFeatures::ObjModel data model. FaceTools::Vis::BaseVisualisation instances are delegates
 * that define ways to visualise the vtkActors generated from the data model. Multiple
 * visualisation layers can be added and removed separately.
 */

#include <FaceTypes.h>
#include <VtkActorCreator.h>    // RVTK
#include <QObject>
#include <QColor>
#include <QPoint>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT FaceView : public QObject
{ Q_OBJECT
public:
    FaceView() : _data(nullptr), _viewer(nullptr) {} // For QMetaType
    FaceView( FM*, FMV*);
    FaceView( const FaceView&);
    FaceView& operator=( const FaceView&);
    virtual ~FaceView();

    inline FM* data() const { return _data;}

    // Upon changing the viewer, all visualisation actors are first removed from the old
    // viewer, and then added to the new viewer. No rebuilding or re-application of
    // visualisations to the vtkActors is undertaken.
    void setViewer( FMV*);
    inline FMV* viewer() const { return _viewer;}

    // Return any previous viewer this view was on (useful when needing to reference FVs on viewer just left).
    inline FMV* pviewer() const { return _pviewer;}

    // Remove and purge all visualisations and rebuild the view models from data. For textured actors
    // only ObjModels with a single material are accepted (for models having multiple materials, use
    // ObjModel::mergeMaterials beforehand). If the ObjModel doesn't have any materials, a warning will
    // be printed to stderr and only non-texture mapped visualisations will be available.
    void reset();

    // Return face actor. Non-const option to allow clients to change properties on
    // the actor not accounted for in the FaceView interface.
    inline vtkActor* actor() { return _actor;}
    inline const vtkActor* actor() const { return _actor;}

    // Remove the given visualisation - pass in null to remove all.
    void remove( BaseVisualisation *vis=nullptr);

    // Add visualisation returning true iff visualisation applied on return.
    bool apply( BaseVisualisation *vis, const QPoint* mc=nullptr);

    // Returns true iff the given visualisation is currently applied.
    bool isApplied( const BaseVisualisation *vis) const;

    // Returns the set of currently applied visualisations.
    const VisualisationLayers& visualisations() const { return _vlayers;}

    // Returns the current exclusive visualisation or null if none set.
    // Only one exclusive visualisation can be set at a time (handled by ActionVisualise).
    BaseVisualisation* exvis() const { return _xvis;}

    // The ObjModel to vtkActor polygon lookups for the face actor created upon bulding.
    // Can be used by visualisations to map information calculated about polygons on the
    // source ObjModel to polygons created for the vtkActor face model.
    const IntIntMap& polyLookups() const { return _fmap;}

    // Returns the visualisation layer that vtkProp belongs to or null.
    BaseVisualisation* layer( const vtkProp*) const;

    // Returns true iff the given point projects to intersect with the face actor.
    bool isPointOnFace( const QPoint& p) const;

    // Project the given 2D point to a location on the 3D surface of the main actor.
    // Returns true iff the point projects to the surface. Ignores all other actors.
    bool projectToSurface( const QPoint&, cv::Vec3f&) const;

    // Set/get the opacity of the face actor.
    void setOpacity( double);
    double opacity() const;

    // Set/get the surface colour of the face actor. Alpha value ignored - use setOpacity.
    void setColour( const QColor&);
    QColor colour() const;

    // Set/get rendering of wireframe on surface.
    void setWireframe( bool);
    bool wireframe() const;

    // Set/get whether texture is applied. Also sets lighting reflection properties for the actor.
    void setTextured( bool);
    bool textured() const;
    bool canTexture() const;    // Returns true iff texturing is available for the actor.

    // Set the active scalars colour mapping of the actor's surface. Scalar visibility turned off if left null.
    void setActiveScalars( ScalarMapping *s=nullptr);
    ScalarMapping* activeScalars() const;

    // Set/get whether backface culling is applied.
    void setBackfaceCulling( bool);
    bool backfaceCulling() const;

private:
    FM *_data;
    vtkActor *_actor;                       // The face actor.
    vtkSmartPointer<vtkTexture> _texture;   // The texture map (if available).
    FMV *_viewer;                           // The viewer this view is attached to.
    FMV *_pviewer;                          // The previous viewer this view was attached to.
    ScalarMapping *_scmap;                  // The active scalar mapping (if not null).
    BaseVisualisation *_xvis;
    VisualisationLayers _vlayers;           // Visualisation layers.
    IntIntMap _fmap;                        // Polygon ID lookups for the actor.

    void applyLayer( BaseVisualisation*, const QPoint*);
};  // end class

}   // end namespace
}   // end namespace

#endif
