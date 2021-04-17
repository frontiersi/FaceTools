/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
 * r3d::Mesh data model. FaceTools::Vis::BaseVisualisation instances are delegates
 * that define ways to visualise the vtkActors generated from the data model. Multiple
 * visualisation layers can be added and removed separately.
 */

#include <FaceTools/FaceTypes.h>
#include <FaceTools/FaceModel.h>
#include <r3dvis/VtkActorCreator.h>
#include <vtkFloatArray.h>
#include <QColor>
#include <QPoint>

namespace FaceTools { namespace Vis {

class ColourVisualisation;

class FaceTools_EXPORT FaceView
{
public:
    FaceView( FM*, FMV*);
    virtual ~FaceView();

    // Try to copy visualisations and other state from the parameter view to this one.
    void copyFrom( const FaceView*);

    inline FM* data() { return _data;}
    inline const FM* data() const { return _data;}

    inline FM::RPtr rdata() const { return _data->scopedReadLock();}
    inline FM::WPtr wdata() { return _data->scopedWriteLock();}

    // Upon changing the viewer, all visualisation actors are first removed from the old
    // viewer, and then added to the new viewer. No rebuilding or re-application of
    // visualisations to the vtkActors is undertaken.
    void setViewer( FMV*);
    inline FMV* viewer() { return _viewer;}
    inline const FMV* viewer() const { return _viewer;}

    // Return any previous viewer this view was on (useful when needing to reference FVs on viewer just left).
    inline FMV* pviewer() { return _pviewer;}
    inline const FMV* pviewer() const { return _pviewer;}

    // Remove and purge all visualisations and rebuild the view models from data. For textured actors
    // only Meshes with a single material are accepted (for models having multiple materials, use
    // Mesh::mergeMaterials beforehand).
    void rebuild();

    // Reset just the normals from generated FaceModelCurvature.
    void resetNormals();

    // Return the main face actor.
    inline const vtkActor* actor() const { return _actor;}
    inline vtkActor* actor() { return _actor;}

    // Return the actor transform matrix for this FaceView.
    inline const vtkMatrix4x4* transformMatrix() const { return _actor->GetMatrix();}
    inline vtkMatrix4x4* transformMatrix() { return actor()->GetMatrix();}

    // Apply the given visualisation and set it to be immediately visible.
    void apply( BaseVisualisation*);
    
    // Returns true iff the given visualisation is applied. Note that it may not be visible since
    // callers can call BaseVisualisation::setVisible to change its visibility state.
    // Call BaseVisualisation::isVisible to check if it's really visible.
    bool isApplied( const BaseVisualisation*) const;

    // Purge the given visualisation from this view.
    void purge( BaseVisualisation*);

    // Returns the set of currently applied visualisations.
    const VisualisationLayers& visualisations() const { return _vlayers;}

    // Returns the visualisation layer that vtkProp belongs to or null.
    const BaseVisualisation* layer( const vtkProp*) const;
    BaseVisualisation* layer( const vtkProp*);

    // Update the actor's transform directly, and propogate to all visualisations.
    void pokeTransform( const vtkMatrix4x4*);

    // Returns true iff the given point projects to intersect with the face actor.
    bool isPointOnFace( const QPoint& p) const;

    // Project the given 2D point to the visible corresponding location on the 3D surface of
    // the main actor. Returns true iff the point projects to the surface. Ignores all other actors.
    // Note that the position vector obtained is transformed.
    bool projectToSurface( const QPoint&, Vec3f&) const;

    // Returns true iff this view overlaps with any other FaceView in its viewer.
    bool overlaps() const;

    // Set/get the opacity of the face actor.
    void setOpacity( float);
    float opacity() const;

    // Get the min/max allowed opacity for the face actor given the applied set of visualisations.
    float minAllowedOpacity() const;
    float maxAllowedOpacity() const;
    void setMinAllowedOpacity( float);
    void setMaxAllowedOpacity( float);

    // Set/get the surface colour of the face actor. Alpha value ignored - use setOpacity.
    void setColour( const QColor&);
    QColor colour() const;
    
    // The standard (default) base colour of untextured models.
    static const QColor BASECOL;

    // Set/get rendering of wireframe on surface.
    void setWireframe( bool);
    bool wireframe() const;

    // Set/get whether texture is applied. Also sets lighting reflection properties for the actor.
    void setTextured( bool);
    bool textured() const;
    bool canTexture() const;    // Returns true iff texturing is available for the actor.

    // Set the actor's active surface mapping. Surface visibility turned off if null.
    void setActiveColours( ColourVisualisation *s=nullptr);
    inline const ColourVisualisation* activeColours() const { return _cv;}
    inline ColourVisualisation* activeColours() { return _cv;}

    void addCellsArray( vtkFloatArray*);
    void addPointsArray( vtkFloatArray*);
    void setActiveCellScalars( const char*);
    void setActivePointScalars( const char*);
    void setActiveCellVectors( const char*);
    void setActivePointVectors( const char*);

    // Set/get whether backface culling is applied.
    void setBackfaceCulling( bool);
    bool backfaceCulling() const;

    // When set true (default is false), polygon normals are generated by VTK on newly generated models
    // which allows for the lighting model to be smoothly graduated over the surface. This can make
    // individual polygons difficult to identify however.
    static void setSmoothLighting( bool v);
    static bool smoothLighting();

    // When true (default is false), scalar mapping is interpolated between vertices.
    static void setInterpolatedShading( bool v);
    static bool interpolatedShading();

private:
    FM *_data;
    vtkSmartPointer<vtkActor> _actor;       // The face actor.
    vtkSmartPointer<vtkTexture> _texture;   // The texture map (if generated).
    vtkSmartPointer<vtkFloatArray> _nrms;   // Surface normals.
    FMV *_viewer;                           // The viewer this view is attached to.
    FMV *_pviewer;                          // The previous viewer this view was attached to.
    ColourVisualisation *_cv;               // The active colour visualisation mapping (if not null).
    QColor _baseCol;
    float _minAllowedOpacity;
    float _maxAllowedOpacity;
    VisualisationLayers _vlayers;           // Visualisation layers.

    static bool s_smoothLighting;
    static bool s_interpolateShading;

    BaseVisualisation* _layer( const vtkProp*) const;
    void _updateSurfaceProperties();
    FaceView( const FaceView&) = delete;
    FaceView& operator=( const FaceView&) = delete;
};  // end class

}}   // end namespaces

#endif
