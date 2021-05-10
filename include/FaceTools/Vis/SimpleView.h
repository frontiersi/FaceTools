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

    // Remove all actors from viewers and clear remove actors.
    void reset();

    void setVisible( bool, ModelViewer*) override;
    bool isVisible() const override { return _visible;}
    bool belongs( const vtkProp*) const override;
    void pokeTransform( const vtkMatrix4x4*) override;

    virtual void setColour( double r, double g, double b, double a=-1);
    virtual void setLineWidth( double);

    const std::vector<vtkSmartPointer<vtkActor> >& actors() const { return _actors;}

    static void setActorColour( vtkActor *actor, double r, double g, double b, double a=-1);

protected:
    vtkActor* addActor( vtkPolyDataAlgorithm*);
    vtkProperty* initActor( vtkSmartPointer<vtkActor>);
    void pokeTransform( vtkActor*, const Mat4f&);
    virtual void pokeTransform( vtkActor*, const vtkMatrix4x4*);

private:
    ModelViewer *_vwr;
    bool _visible;
    std::vector<vtkSmartPointer<vtkActor> > _actors;
};  // end class

}}   // end namespaces

#endif
