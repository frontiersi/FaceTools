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

#include <Vis/SimpleView.h>
#include <VtkTools.h>
#include <vtkProperty.h>
using FaceTools::Vis::SimpleView;
using FaceTools::ModelViewer;


SimpleView::SimpleView() : _vwr(nullptr), _visible(false) {}


SimpleView::~SimpleView() { setVisible(false, _vwr);}


void SimpleView::initActor( vtkSmartPointer<vtkActor> actor)
{
    _actors.push_back(actor);
    vtkProperty* prop = actor->GetProperty();
    prop->SetAmbient( 1.0);
    prop->SetDiffuse( 0.0);
    prop->SetSpecular( 0.0);
}   // end initActor


void SimpleView::setColour( double r, double g, double b, double a)
{
    for ( vtkActor* actor : _actors)
    {
        vtkProperty* prop = actor->GetProperty();
        prop->SetColor( r, g, b);
        prop->SetOpacity( a);
    }   // end for
}   // end setColour


void SimpleView::pokeTransform( const vtkMatrix4x4* d)
{
    for ( vtkActor* actor : _actors)
        pokeTransform( actor, d);
}   // end pokeTransform


void SimpleView::pokeTransform( vtkActor* actor, const cv::Matx44d& m)
{
    vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK(m);
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
