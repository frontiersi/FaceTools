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

#ifndef FACE_TOOLS_SIMPLE_VIEW_H
#define FACE_TOOLS_SIMPLE_VIEW_H

/**
 * For non textured actors such as lines and boxes etc.
 */

#include "ViewInterface.h"
#include <vtkProperty.h>
#include <vtkPolyDataAlgorithm.h>

namespace FaceTools { namespace Vis {

class FaceTools_EXPORT SimpleView : public ViewInterface
{
public:
    SimpleView();
    virtual ~SimpleView();

    void setLineStipplingEnabled( bool);    // off by default

    void setColour( double r, double g, double b, double a=-1) override;
    void setVisible( bool, ModelViewer*) override;
    bool isVisible() const override { return _visible;}
    bool belongs( const vtkProp*) const override;
    void pokeTransform( const vtkMatrix4x4*) override;

    void setLineWidth( double);

    const std::vector<vtkSmartPointer<vtkActor> >& actors() const { return _actors;}

    static void setActorColour( vtkActor *actor, double r, double g, double b, double a=-1);

protected:
    vtkActor* addActor( vtkPolyDataAlgorithm*);
    vtkProperty* initActor( vtkSmartPointer<vtkActor>);
    void pokeTransform( vtkActor*, const Mat4f&);
    virtual void pokeTransform( vtkActor*, const vtkMatrix4x4*);

    std::vector<vtkSmartPointer<vtkActor> > _actors;

private:
    ModelViewer *_vwr;
    bool _visible;
    static vtkSmartPointer<vtkTexture> s_stippleTexture;

    void _setStippleTextureCoords( vtkActor*);
    vtkSmartPointer<vtkTexture> _stippleTexture();

    SimpleView( const SimpleView&) = delete;
    void operator=( const SimpleView&) = delete;
};  // end class

}}   // end namespaces

#endif
