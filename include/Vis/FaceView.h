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

#ifndef FACE_TOOLS_FACE_VIEW_H
#define FACE_TOOLS_FACE_VIEW_H

/**
 * The container for coordinating the generation and application of visualisations for a
 * RFeatures::ObjModel data model. FaceTools::Vis::BaseVisualisation instances are delegates
 * that define ways to visualise the vtkActors generated from the data model. Multiple
 * visualisation layers can be added and removed separately.
 */

#include <FaceTools_Export.h>
#include <VtkActorCreator.h>    // RVTK
#include <QColor>

namespace FaceTools {
class ModelViewer;
class FaceControl;

namespace Vis {
class BaseVisualisation;

class FaceTools_EXPORT FaceView
{
public:
    explicit FaceView( const FaceControl*);
    virtual ~FaceView();

    // Upon changing the viewer, all visualisation actors are first removed from the old
    // viewer, and then added to the new viewer. No rebuilding or re-application of
    // visualisations to the vtkActors is undertaken.
    void setViewer( ModelViewer*);
    ModelViewer* viewer() const { return _viewer;}  // May be null

    // Remove all visualisations and rebuild the view models from the data model. For textured actors
    // only ObjModels with a single material are accepted (for models having multiple materials, use
    // ObjModel::mergeMaterials beforehand). If the ObjModel doesn't have any materials, a warning will
    // be printed to stderr and only non-texture mapped visualisations will be available.
    void reset();

    // Remove the given visualisation, or remove all if parameter left as null.
    void remove( BaseVisualisation *vis=nullptr);

    // Apply a visualisation and add the visualisation to the current viewer.
    // Actors relating to the given visualisation are first removed before being re-added.
    // If the given visualisation is "exclusive", it will replace any existing "exclusive"
    // visualisation (the actors of the existing exclusive visualisation will be removed).
    // Non-exclusive visualisations are added without replacing existing ones.
    bool apply( BaseVisualisation *vis=nullptr);

    bool isApplied( const BaseVisualisation *vis) const;

    // Returns the set of currently applied visualisations.
    const std::unordered_set<BaseVisualisation*> visualisations() const { return _vlayers;}

    // Returns the current exclusive visualisation (null if none set).
    BaseVisualisation* exclusiveVisualisation() const { return _visx;}

    // Allow a visualisation to use either the surface or the textured actors.
    vtkActor* surfaceActor() const { return _sactor;}
    vtkActor* textureActor() const { return _tactor;}

    // The ObjModel to vtkActor polygon lookups for the surface model created upon bulding.
    // Can be used by visualisations to map information calculated about polygons on the
    // source ObjModel to polygons created for the vtkActor surface model.
    const IntIntMap& polyLookups() const { return _fmap;}

    // Returns the visualisation that vtkProp belongs to or null.
    BaseVisualisation* belongs( const vtkProp*) const;

    // Set/get the opacity of the surface and texture models.
    void setOpacity( double);
    double opacity() const;

    // Set/get the surface colour that applies to the surface actor only.
    // The colour's alpha value is ignored - use setOpacity instead.
    void setColour( const QColor&);
    QColor colour() const;

    // Set/get whether backface culling is applied.
    void setBackfaceCulling( bool);
    bool backfaceCulling() const;

    // Returns the transform matrix for the actor of the current exclusive visualisation.
    // This will be the identity matrix unless the actor has been moved via pokeTransform.
    vtkSmartPointer<vtkMatrix4x4> userTransform() const;

    // Transform the visualisation actors using the given matrix.
    void pokeTransform( const vtkMatrix4x4*, bool transEx=true);

    // Fix the current transform for all visualisation actors and cause
    // the userTransform to become reset to the identity matrix.
    void fixTransform();

private:
    const FaceControl *_fc;
    ModelViewer *_viewer;                               // The viewer being rendered to.
    BaseVisualisation *_visx;                           // The exclusive visualisation.
    std::unordered_set<BaseVisualisation*> _vlayers;    // Visualisation layers.
    vtkSmartPointer<vtkActor> _sactor, _tactor;         // The surface model and texture model.
    IntIntMap _fmap;                                    // Polygon ID lookups for the surface model.
    FaceView( const FaceView&) = delete;
    void operator=( const FaceView&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
