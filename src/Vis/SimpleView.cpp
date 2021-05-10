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

#include <Vis/SimpleView.h>
#include <r3dvis/VtkTools.h>
#include <vtkImageData.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
using FaceTools::Vis::SimpleView;
using FaceTools::ModelViewer;
using FaceTools::Mat4f;


SimpleView::SimpleView() : _vwr(nullptr), _visible(false) {}


SimpleView::~SimpleView() { setVisible(false, _vwr);}


void SimpleView::reset()
{ 
    setVisible(false, _vwr);
    _actors.clear();
}   // end reset


vtkActor* SimpleView::addActor( vtkPolyDataAlgorithm *src)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( src->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper( mapper);
    actor->SetPickable( false);
    initActor(actor);
    return actor;
}   // end addActor


vtkProperty* SimpleView::initActor( vtkSmartPointer<vtkActor> actor)
{
    _actors.push_back(actor);
    vtkProperty* prop = actor->GetProperty();
    prop->SetAmbient( 1.0);
    prop->SetDiffuse( 0.0);
    prop->SetSpecular( 0.0);
    prop->SetOpacity( 0.99);
    return prop;
}   // end initActor


void SimpleView::setColour( double r, double g, double b, double a)
{
    for ( vtkActor* actor : _actors)
        setActorColour( actor, r, g, b, a);
}   // end setColour


void SimpleView::setActorColour( vtkActor *actor, double r, double g, double b, double a)
{
    r = std::max( 0.0, std::min( r, 1.0));
    g = std::max( 0.0, std::min( g, 1.0));
    b = std::max( 0.0, std::min( b, 1.0));
    vtkProperty* prop = actor->GetProperty();
    prop->SetColor( r, g, b);
    if ( a >= 0.0 && a <= 1.0)
        prop->SetOpacity( a);
}   // end setActorColour


void SimpleView::setLineWidth( double lw)
{
    for ( vtkActor* actor : _actors)
        actor->GetProperty()->SetLineWidth( lw);
}   // end setLineWidth


void SimpleView::pokeTransform( const vtkMatrix4x4* d)
{
    for ( vtkActor* actor : _actors)
        pokeTransform( actor, d);
}   // end pokeTransform


void SimpleView::pokeTransform( vtkActor* actor, const Mat4f& m)
{
    vtkSmartPointer<vtkMatrix4x4> vm = r3dvis::toVTK(m);
    pokeTransform( actor, vm);
}   // end pokeTransform


void SimpleView::pokeTransform( vtkActor* actor, const vtkMatrix4x4* d)
{
    actor->PokeMatrix( const_cast<vtkMatrix4x4*>(d));
}   // end pokeTransform


void SimpleView::setVisible( bool visible, ModelViewer* vwr)
{
    if ( !visible)
    {
        for ( vtkActor* actor : _actors)
        {
            if ( _vwr)
                _vwr->remove(actor);
            if ( vwr)
                vwr->remove(actor);
        }   // end for
        _visible = false;
    }   // end if

    _vwr = vwr;
    
    if ( visible && _vwr)
    {
        for ( vtkActor* actor : _actors)
            _vwr->add(actor);
        _visible = true;
    }   // end if
}   // end setVisible


bool SimpleView::belongs( const vtkProp* prop) const
{
    for ( vtkActor* actor : _actors)
        if ( prop == actor)
            return true;
    return false;
}   // end belongs
