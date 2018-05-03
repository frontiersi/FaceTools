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
 * that define ways to visualise the vtkActors generated from the data model passed in to
 * rebuild. Multiple visualisation layers can be added and removed separately. See related
 * classes for further info:
 * FaceTools::Vis::VisualisationInterface
 * FaceTools::Vis::BaseVisualisation
 * FaceTools::Action::ActionVisualise
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
    ModelViewer* viewer() const { return _viewer;}  // May be NULL

    // Rebuild the view models from the data model and reapply the visualisations (first removing
    // previous view actors). For textured actors only ObjModels with a single material are
    // accepted (for models having multiple materials, use ObjModel::mergeMaterials beforehand).
    // If the ObjModel doesn't have any materials, a warning will be printed to stderr and only
    // non-texture mapped visualisations will be available.
    void rebuild();

    // Remove the given visualisation, or remove all if parameter left as NULL.
    void remove( BaseVisualisation *vis=NULL);

    // Apply (or re-apply) a visualisation and add the visualisation to the current viewer.
    // Actors relating to the given visualisation are first removed before being re-added.
    // If the visualisation parameter is left as NULL, the existing visualisation(s) are
    // re-applied. If the given visualisation is "exclusive", it will replace any existing
    // "exclusive" visualisation (the actors of the existing exclusive visualisation will
    // be removed). Non-exclusive visualisations are added without replacing existing ones.
    // Note that all applied visualisations 
    bool apply( BaseVisualisation *vis=NULL);

    // Returns the set of currently applied visualisations.
    const std::unordered_set<BaseVisualisation*> visualisations() const { return _vlayers;}

    // Allow a visualisation to use either the surface or the textured actors.
    vtkSmartPointer<vtkActor> surfaceActor() const { return _sactor;}
    vtkSmartPointer<vtkActor> textureActor() const { return _tactor;}

    // The ObjModel to vtkActor polygon lookups for the surface model created upon bulding.
    // Can be used by visualisations to map information calculated about polygons on the
    // source ObjModel to polygons created for the vtkActor surface model.
    const IntIntMap& polyLookups() const { return _fmap;}

    // Returns the visualisation that vtkProp belongs to or NULL.
    const BaseVisualisation* belongs( const vtkProp*) const;

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

    // Transform the actors and any visualisations using the given matrix. This is a "hard"
    // transform in that it updates the poly data underlying the actors. If the given matrix
    // is NULL, the transform will use the actor's current user transform which is nominally
    // the identity matrix unless the actors have been moved around (typically via interactor).
    // The returned matrix will be a copy of the actor's user transformation matrix prior
    // to applying the transform to the data.
    //
    // If the given matrix is NOT null, the actor's internal transformation matrix will be
    // ignored and the poly data filtered to update according to the provided matrix. In
    // this case, the returned matrix will simply be a copy of the provided matrix.
    // On return, the actor's user transformation matrix will be the identity matrix.
    const vtkMatrix4x4* transform( const vtkMatrix4x4 *t=NULL);

private:
    const FaceControl *_fc;
    ModelViewer *_viewer;                               // The viewer being rendered to.
    BaseVisualisation *_visx;                           // The exclusive visualisation.
    std::unordered_set<BaseVisualisation*> _vlayers;    // Visualisation layers.
    vtkSmartPointer<vtkActor> _sactor, _tactor;         // The surface and texture models.
    IntIntMap _fmap;                                    // Polygon ID lookups for the surface model.
    FaceView( const FaceView&);       // No copy
    void operator=( const FaceView&); // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
