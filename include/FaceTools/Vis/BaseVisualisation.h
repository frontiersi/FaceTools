/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include "FaceView.h"

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT BaseVisualisation
{
public:
    BaseVisualisation(){}
    virtual ~BaseVisualisation(){}

    virtual const char* name() const { return "BaseVisualisation (virtual)";}

    virtual bool isAvailable( const FV*, const QPoint*) const { return true;}

    // Visualisations that are toggled (default) can be turned on and off as layers over the base view.
    // A non-toggled visualisation is mutually exclusive wrt any other non-toggled visualisation.
    virtual bool isToggled() const { return true;}

    // By default, visualisations are applied to all views of all models in the viewer of the selected model.
    // This behaviour can be changed by overriding applyToAllInViewer to return false, in which case
    // the visualisation is applied only to the selected model's view. If applyToAllViewers is overridden
    // to return true, the selected set of FaceViews is first expanded to be the superset
    // of all FaceViews owned by every selected FaceModel.
    virtual bool applyToAllInViewer() const { return true;}
    virtual bool applyToAllViewers() const { return false;}

    // Return true iff given prop relating to the given FV belongs to this visualisation.
    virtual bool belongs( const vtkProp*, const FV*) const { return false;}
    //return fv && p && fv->actor() == p

    // Cause this visualisation to update its actor transforms to match the FaceView actor transform.
    // given FaceView transform. This may not be the same as the FaceModel or data transform since
    // the FaceView actor can be updated interactively and only after interaction finishes is the
    // transform for the mesh updated to match.
    virtual void syncWithViewTransform( const FV*){}

    // Set the visibility of applied visualisations.
    virtual void setVisible( FV*, bool) {}
    virtual bool isVisible( const FV*) const { return false;}

    // Called from ActionVisualise::checkState and only if this visualisation is currently
    // visible against the given view in order to allow this visualisation to update itself
    // according to the application's current state.
    virtual void refreshState( const FV*){}

    // Specify the minimum and maximum allowed opacity of the face when this
    // visualisation is applied. If multiple visualisations are applied, the
    // min/max settings are chosen to be the highest min and the lowest max
    // over all visible visualisations.
    virtual float minAllowedOpacity() const { return 0.0f;}
    virtual float maxAllowedOpacity() const { return 1.0f;}

    // Create actors for the visualisation if necessary.
    virtual void apply( const FV*, const QPoint* p=nullptr) {}

    // Destroy any created actors.
    virtual void purge( const FV*) {}

private:
    BaseVisualisation( const BaseVisualisation&) = delete;
    void operator=( const BaseVisualisation&) = delete;
};  // end class


class FaceTools_EXPORT TextureVisualisation : public BaseVisualisation
{
public:
    virtual const char* name() const { return "TextureVisualisation";}
    void setVisible( FV *fv, bool v) override { fv->setTextured(v);}
    bool isVisible( const FV* fv) const override { return fv->textured();}
};  // end class


class FaceTools_EXPORT WireframeVisualisation : public BaseVisualisation
{
public:
    virtual const char* name() const { return "WireframeVisualisation";}
    void setVisible( FV *fv, bool v) override { fv->setWireframe( v);}
    bool isVisible( const FV *fv) const override { return fv->wireframe();}
};  // end class

}}   // end namespaces

#endif
