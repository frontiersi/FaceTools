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

namespace FaceTools {
namespace Action {
class ActionVisualise;
class FaceAction;
}   // end namespace

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

    // Returns true iff this visualisation is currently applied in the given view.
    bool isApplied( const FaceControl*) const;

    // Return true iff the given prop relating to the given FaceControl belongs to
    // this visualisation. Typically, visualisations do not define extra actors
    // so the default implementation defaults to returning false.
    virtual bool belongs( const vtkProp*, const FaceControl*) const { return false;}

    // Derived types still need to provide overrides for:
    // void apply( const FaceControl*)
    // void addActors( const FaceControl*)
    // void removeActors( const FaceControl*)

protected:
    // An applied visualisation may need to perform further activation upon selection.
    virtual void onSelected( const FaceControl*){}

    // Specifiy whether this visualisation should respond to DATA/CALC type change events.
    virtual bool respondData() const { return false;}
    virtual bool respondCalc() const { return false;}

    // Respond to a ChangeEvent by the given action for the given face. This function is
    // only ever called if respondData or respondCalc are overridden to return true; the
    // action that makes the respective change is passed in as parameter to this function.
    // Derived types only need to override this function if overriding respondData/Calc.
    virtual void respondTo( const Action::FaceAction*, const FaceControl*){}

    // Transform this visualisation's defined actors (if any) for the given FaceControl. Derived
    // types only need to override if they define vtkProp3D actors that must respond to movements
    // of the main FaceView actors in the given FaceControl.
    virtual void transform( const FaceControl*, const vtkMatrix4x4*){}

    // Destroy any cached data relating to the given FaceControl.
    virtual void purge( const FaceControl*){}

    friend class Action::ActionVisualise;
    friend class FaceView;

private:
    const QString _dname;           // Display name
    const QIcon *_icon;             // Display icon
    const QKeySequence *_keys;      // Key shortcut

    BaseVisualisation( const BaseVisualisation&);
    void operator=( const BaseVisualisation&);
};  // end class


class FaceTools_EXPORT TextureVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    TextureVisualisation( const QString& displayName="Texture",
                          const QIcon &icon=QIcon(":/icons/TEXTURE_VIS"),
                          const QKeySequence &keys=QKeySequence(Qt::Key_1))
        : BaseVisualisation( displayName, icon, keys) {}

    // Nothing needs to be done to apply the visualisation since the textured actor is used as is.
    void apply( const FaceControl*) override {}

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

    void apply( const FaceControl*) override; // Sets the scalar visibility to false and the representation to surface.

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
    void apply( const FaceControl*) override;   // Sets representation to vertices
};  // end class


class FaceTools_EXPORT WireframeVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    WireframeVisualisation( const QString& displayName="Wireframe",
                            const QIcon &icon=QIcon(":/icons/WIREFRAME_VIS"),
                            const QKeySequence &keys=QKeySequence(Qt::Key_4))
        : SurfaceVisualisation( displayName, icon, keys) {}
    void apply( const FaceControl* fc) override;    // Sets representation to wireframe
};  // end class


}   // end namespace
}   // end namespace

#endif
