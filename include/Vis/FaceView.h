/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <QColor>
#include <QPoint>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT FaceView
{
public:
    FaceView( FM*, FMV*);
    virtual ~FaceView();

    // Copy visualisations and other state from the parameter view to this one.
    // Only works if the parameter view and this one share the same underlying
    // FaceModel, otherwise false is returned.
    bool copyFrom( const FaceView*);

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

    // Apply the given visualisation and make it visible.
    // After applying, a visualisation's visibility is controlled directly
    // through its interface using BaseVisualisation::setVisible.
    void apply( BaseVisualisation*, const QPoint* mc=nullptr);

    // Ask the given visualisation to purge before removing it from this view.
    // If the given visualisation decides not to purge (i.e. returning false from BaseVisualisation::purge)
    // then the visualisation will remain visible and attached to this view. Only if the event is Event::NONE
    // will the visualisation be set hidden and removed from this view. This function returns true iff the
    // visualisation was purged.
    bool purge( BaseVisualisation*, Action::Event e=Action::Event::NONE);

    // Returns the set of currently applied visualisations.
    const VisualisationLayers& visualisations() const { return _vlayers;}

    // Returns the visualisation layer that vtkProp belongs to or null.
    BaseVisualisation* layer( const vtkProp*) const;

    // When updating the actor's transform directly, call to propogate the transform to all visualisations.
    void syncVisualisationsToViewTransform();

    // Synchronise the tranform matrix of all visualisations and the FaceView actor to the model transform.
    void syncToModelTransform();

    // Returns true iff the given point projects to intersect with the face actor.
    bool isPointOnFace( const QPoint& p) const;

    // Project the given 2D point to the visible corresponding location on the 3D surface of
    // the main actor. Returns true iff the point projects to the surface. Ignores all other actors.
    // If useUntransformed is true, the out point will be set to correspond with the position on the
    // surface that is untransformed by the view's transform matrix. Note that the view's transform
    // matrix should always be in sync with the model transform matrix so using either is fine.
    bool projectToSurface( const QPoint&, cv::Vec3f&, bool useUntransformed=false) const;

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

    // Set the active surface mapping of the actor's surface. Surface visibility turned off if left null.
    void setActiveSurface( SurfaceMetricsMapper *s=nullptr);
    SurfaceMetricsMapper* activeSurface() const;

    // Set/get whether backface culling is applied.
    void setBackfaceCulling( bool);
    bool backfaceCulling() const;

private:
    FM *_data;
    vtkActor *_actor;                       // The face actor.
    vtkSmartPointer<vtkTexture> _texture;   // The texture map (if available).
    FMV *_viewer;                           // The viewer this view is attached to.
    FMV *_pviewer;                          // The previous viewer this view was attached to.
    SurfaceMetricsMapper *_smm;             // The active surface mapping (if not null).
    QColor _baseCol;
    BaseVisualisation *_xvis;
    VisualisationLayers _vlayers;           // Visualisation layers.

    void _updateModelLighting();
    FaceView( const FaceView&) = delete;
    FaceView& operator=( const FaceView&) = delete;
};  // end class

}}   // end namespaces

#endif
