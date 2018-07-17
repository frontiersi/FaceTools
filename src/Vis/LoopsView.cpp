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

#include <FaceTools.h>
#include <LoopsView.h>
#include <VtkActorCreator.h>    // RVTK
#include <VtkTools.h>           // RVTK
#include <vtkProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LoopsView;
using FaceTools::ModelViewer;


// public
LoopsView::LoopsView( float lw, float r, float g, float b)
    : _viewer(nullptr), _visible(false), _lineWidth(lw), _colour(r,g,b)
{
}   // end ctor


LoopsView::~LoopsView()
{
    removeActors();
}   // end dtor


// public
void LoopsView::removeActors()
{
    setVisible( false, _viewer);
    std::for_each( std::begin(_actors), std::end(_actors), [](auto d){ d->Delete();});
    _actors.clear();
}   // end removeActors


// public
void LoopsView::addLoop( const std::list<cv::Vec3f>& line)
{
    vtkActor* actor = RVTK::VtkActorCreator::generateLineActor( line, true);  // Joins ends
    actor->SetPickable(false);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( _lineWidth);
    property->SetColor( _colour[0], _colour[1], _colour[2]);

    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular( 0.0);
    
    _actors.insert(actor);
}   // end addLoop


void LoopsView::addPoints( const std::vector<cv::Vec3f>& pts)
{
    vtkActor* actor = RVTK::VtkActorCreator::generatePointsActor( pts);
    actor->SetPickable(false);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToPoints();
    property->SetPointSize( _lineWidth);
    property->SetColor( _colour[0], _colour[1], _colour[2]);

    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular( 0.0);
    
    _actors.insert(actor);
}   // end addPoints


// public
void LoopsView::setVisible( bool visible, ModelViewer* viewer)
{
    if ( _viewer != viewer && _viewer)
        std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->remove(a);});
    _visible = false;
    _viewer = viewer;
    if ( _viewer)
    {
        if ( visible)
            std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->add(a);});
        else
            std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->remove(a);});
        _visible = visible;
    }   // end if
}   // end setVisible


// public
void LoopsView::pokeTransform( const vtkMatrix4x4* vm)
{
    vtkMatrix4x4* m = const_cast<vtkMatrix4x4*>(vm);
    std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ a->PokeMatrix(m);});
}   // end pokeTransform


// public
void LoopsView::fixTransform()
{
    std::for_each( std::begin(_actors), std::end(_actors), [](auto a){ RVTK::transform( a, a->GetMatrix());});
}   // end fixTransform
