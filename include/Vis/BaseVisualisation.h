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

#ifndef FACE_TOOLS_VIS_BASE_VISUALISATION_H
#define FACE_TOOLS_VIS_BASE_VISUALISATION_H

#include <FaceTypes.h>
#include <vtkMatrix4x4.h>
#include <vtkProp.h>
#include <cassert>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BaseVisualisation
{
public:
    BaseVisualisation(){}
    virtual ~BaseVisualisation(){}

    virtual bool isAvailable( const FM*) const { return true;}
    virtual bool isAvailable( const FV*, const QPoint*) const;

    // Visualisations that are toggled (default) can be turned on and off as layers over the base view.
    // A non-toggled visualisation is mutually exclusive wrt any other non-toggled visualisation.
    virtual bool isToggled() const { return true;}

    // If a visualisation is non-toggled, then it is exclusive by default. Visualisations
    // that toggle on/off can also enforce exclusivity when toggling on. This is the case
    // with the TextureVisualisation (declared in this file).
    virtual bool isExclusive() const { return false;}

    // By default, visualisations are applied to all FaceViews in the selected FaceView(s) viewer(s).
    // This behaviour can be changed by overriding applyToAllInViewer to return false, in which case
    // the visualisation is applied only to the selected FaceView(s). If applyToSelectedModel is
    // overridden to return true, the selected set of FaceViews is first expanded to be the superset
    // of all FaceViews owned by every selected FaceModel.
    virtual bool applyToAllInViewer() const { return true;}
    virtual bool applyToSelectedModel() const { return false;}

    // Return true iff the given prop relating to the given FV belongs to
    // this visualisation. Typically, visualisations do not apply extra actors
    // so the default implementation defaults to returning false.
    virtual bool belongs( const vtkProp*, const FV*) const { return false;}

    // Cause this visualisation to update its actor transforms to match the FaceView
    // given FaceView transform (this is not the same as the FaceModel or data transform).
    virtual void syncToViewTransform( const FV*, const vtkMatrix4x4*){}

    // Set the visibility of the visualisation if applied.
    virtual void setVisible( FV*, bool) {}

    // Get the visibility of the visualisation (should always return false if not applied).
    virtual bool isVisible( const FV*) const { return false;}

    // Called from ActionVisualise::checkState and only if this visualisation is currently
    // visible against the given view in order to allow this visualisation to update itself
    // according to the application's current state.
    virtual void checkState( const FV*){}

    // Apply the visualisation for the given FaceView (add associated actors to viewer).
    virtual void apply( FV*, const QPoint* p=nullptr) = 0;

    // Remove the visualisation for the given FaceView (remove associated actors from viewer).
    virtual bool purge( FV*, Action::Event) = 0;

private:
    BaseVisualisation( const BaseVisualisation&) = delete;
    void operator=( const BaseVisualisation&) = delete;
};  // end class


class FaceTools_EXPORT TextureVisualisation : public BaseVisualisation
{
public:
    bool isExclusive() const override { return true;}

    bool belongs( const vtkProp*, const FV*) const override; // Returns true iff fc->view()->isFace(prop)

    void apply( FV*, const QPoint*) override;
    bool purge( FV*, Action::Event) override;
    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;
};  // end class


class FaceTools_EXPORT WireframeVisualisation : public BaseVisualisation
{
public:
    bool belongs( const vtkProp*, const FV*) const override; // Returns true iff fc->view()->isFace(prop)

    void apply( FV*, const QPoint*) override;
    bool purge( FV*, Action::Event) override;
    void setVisible( FV*, bool) override;
    bool isVisible( const FV*) const override;
};  // end class

}}   // end namespaces

#endif
