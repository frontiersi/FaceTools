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

#ifndef FACE_TOOLS_VIS_BASE_VISUALISATION_H
#define FACE_TOOLS_VIS_BASE_VISUALISATION_H

/**
 * The types defined in this header are the basic exclusive visualisation classes.
 * Non-exclusive visualisations should derive from BaseVisualisation and override
 * isExclusive to return false.
 * PointsVisualisation and WireframeVisualisation are both kinds of SurfaceVisualisation
 * which is by default the surface actor returned from FaceView. TextureVisualisation
 * uses the textured actor returned from FaceView.
 * Visualisations that map information to the surface will usually want to derive
 * from SurfaceVisualisation rather than BaseVisualisation.
 */

#include "VisualisationInterface.h"
#include <ChangeEvents.h>
#include <vtkMatrix4x4.h>
#include <vtkProp.h>
#include <cassert>

namespace FaceTools {
namespace Action { class ActionVisualise;}

namespace Vis {
class FaceView;

// Derived classes should inherit BaseVisualisation instead of VisualisationInterface.
class FaceTools_EXPORT BaseVisualisation : public VisualisationInterface
{ Q_OBJECT
public:
    BaseVisualisation( const QString& displayName, const QIcon&, const QKeySequence&);
    BaseVisualisation( const QString& displayName, const QIcon&);
    explicit BaseVisualisation( const QString& displayName);
    virtual ~BaseVisualisation();

    QString getDisplayName() const override { return _dname;}
    const QIcon* getIcon() const override { return _icon;}
    const QKeySequence* getShortcut() const override { return _keys;}

    bool isExclusive() const override { return true;}
    bool isAvailable( const FaceModel*) const override { return true;}
    bool isAvailable( const FaceControl*, const QPoint* p=nullptr) const override { return true;}

    // Should this visualisation be presented as user actionable?
    virtual bool isVisible() const { return true;}

    // By default visualisations apply to all models within a viewer.
    // If singleModel is overridden, the visualisation is applied to all
    // views of the currently selected model in any viewer. If singleView
    // is overridden, the visualisation is applied just to the currently
    // selected view. Note that these cannot both be logically true.
    // If both are true, singleModel takes precedence.
    virtual bool singleModel() const { return false;}
    virtual bool singleView() const { return false;}

    // Return true iff the given prop relating to the given FaceControl belongs to
    // this visualisation. Typically, visualisations do not define extra actors
    // so the default implementation defaults to returning false.
    virtual bool belongs( const vtkProp*, const FaceControl*) const { return false;}

    // Returns true if this visualisation is applied to the given FaceControl.
    bool isApplied( const FaceControl*) const;

    // Derived types still need to provide overrides for:
    // bool apply( const FaceControl*, const QPoint*);
    // void addActors( const FaceControl*)
    // void removeActors( const FaceControl*)

signals:
    void onAvailable( const FaceModel*, bool);  // Inform of change in data availability for visualisation.

protected:
    // Poke the matrix transform for this visualisation's actors into a temporary state (called by FaceView).
    virtual void pokeTransform( const FaceControl*, const vtkMatrix4x4*){}

    // Fix the current position of the actors for this visualisation
    // according to their current transform matrix (called by FaceView).
    virtual void fixTransform( const FaceControl*){}

    // Destroy any cached data associated with the given FaceControl (called by ActionVisualise).
    virtual void purge( const FaceControl*){}
    // Destroy any cached data associated with the given FaceModel (called by ActionVisualise).
    virtual void purge( const FaceModel*){}

    // Descendent classes should add events to the given set that this visualisation will be purged for.
    // NB visualisations will always be purged for GEOMETRY_CHANGE so there's no need to add that one.
    virtual void addPurgeEvents( Action::ChangeEventSet&) const {}

    // Similarly for reapplying the visualisation (without necessarily purging).
    virtual void addProcessEvents( Action::ChangeEventSet&) const {}

    // Specifies if application of this visualisation should be automatically
    // synchronised with a FaceControl's ready (selected) status.
    virtual bool applyOnReady() const { return false;}

    friend class Action::ActionVisualise;
    friend class FaceView;

private:
    const QString _dname;           // Display name
    const QIcon *_icon;             // Display icon
    const QKeySequence *_keys;      // Key shortcut

    BaseVisualisation( const BaseVisualisation&) = delete;
    void operator=( const BaseVisualisation&) = delete;
};  // end class


class FaceTools_EXPORT TextureVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    TextureVisualisation( const QString& displayName="Texture",
                          const QIcon &icon=QIcon(":/icons/TEXTURE_VIS"),
                          const QKeySequence &keys=QKeySequence(Qt::Key_1))
        : BaseVisualisation( displayName, icon, keys) {}

    // Hides the scalar legend.
    bool apply( const FaceControl*, const QPoint* p=nullptr) override;

    // Add and remove the texture actor.
    void addActors( const FaceControl* fc) override;
    void removeActors( const FaceControl* fc) override;

    // Only available if the model has a single material (must merge materials).
    bool isAvailable( const FaceModel*) const override;

protected:
    bool belongs( const vtkProp *p, const FaceControl* fc) const;   // Test prop against texture actor only.
};  // end class


class FaceTools_EXPORT SurfaceVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    SurfaceVisualisation( const QString& displayName, const QIcon& icon, const QKeySequence& keys)
        : BaseVisualisation( displayName, icon, keys) {}
    SurfaceVisualisation( const QString& displayName, const QIcon& icon)
        : BaseVisualisation( displayName, icon) {}
    explicit SurfaceVisualisation( const QString& displayName)
        : BaseVisualisation( displayName) {}
    SurfaceVisualisation()  // Default constructor for plain surface.
        : BaseVisualisation( "Surface", QIcon(":/icons/SURFACE_VIS"), QKeySequence(Qt::Key_2)) {}
 
    // Sets the scalar visibility to false and the representation to surface.
    // Hides the scalar legend.
    bool apply( const FaceControl*, const QPoint* p=nullptr) override;

    // Add and remove the surface actor.
    void addActors( const FaceControl* fc) override;
    void removeActors( const FaceControl* fc) override;

protected:
    bool belongs( const vtkProp *p, const FaceControl* fc) const;   // Test prop against surface actor only.
};  // end class


class FaceTools_EXPORT PointsVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    PointsVisualisation( const QString& displayName="Points",
                         const QIcon &icon=QIcon(":/icons/POINTS_VIS"),
                         const QKeySequence &keys=QKeySequence(Qt::Key_3))
        : SurfaceVisualisation( displayName, icon, keys) {}
    bool apply( const FaceControl*, const QPoint* p=nullptr) override;   // Sets representation to vertices
};  // end class


class FaceTools_EXPORT WireframeVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    WireframeVisualisation( const QString& displayName="Wireframe",
                            const QIcon &icon=QIcon(":/icons/WIREFRAME_VIS"),
                            const QKeySequence &keys=QKeySequence(Qt::Key_4))
        : SurfaceVisualisation( displayName, icon, keys) {}
    bool apply( const FaceControl* fc, const QPoint* p=nullptr) override;    // Sets representation to wireframe
};  // end class


}   // end namespace
}   // end namespace

#endif
