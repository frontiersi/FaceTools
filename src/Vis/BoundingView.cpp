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

#include <BoundingView.h>
#include <VtkActorCreator.h>    // RVTK
#include <VtkTools.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <algorithm>
using FaceTools::Vis::BoundingView;
using FaceTools::ModelViewer;


// public
BoundingView::BoundingView( const std::vector<cv::Vec6d>& bounds, float lw, float r, float g, float b)
    : _viewer(nullptr), _visible(false), _pickable(false)
{
    for ( const cv::Vec6d& cb : bounds)
    {
        vtkCubeSource* source = vtkCubeSource::New();
        source->SetBounds( &cb[0]);
        _sources.push_back(source);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection( source->GetOutputPort());
        vtkActor* actor = vtkActor::New();
        actor->SetMapper(mapper);

        vtkProperty* property = actor->GetProperty();

        property->SetRepresentationToWireframe();
        property->SetRenderLinesAsTubes(false);
        property->SetLineWidth( lw);
        property->SetColor( r, g, b);

        // Don't want cuboid actor to be affected by directional lighting
        property->SetAmbient( 1.0);
        property->SetDiffuse( 0.0);
        property->SetSpecular( 0.0);

        actor->SetPickable(false);
        _actors.push_back(actor);
    }   // end for
}   // end ctor


BoundingView::~BoundingView()
{
    setVisible( false, _viewer);
    std::for_each( std::begin(_actors), std::end(_actors), [](auto a){ a->Delete();});
    std::for_each( std::begin(_sources), std::end(_sources), [](auto s){ s->Delete();});
}   // end dtor


void BoundingView::setPickable( bool v)
{
    std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ a->SetPickable(v);});
    _pickable = v;
}   // end setPickable


bool BoundingView::pickable() const { return _pickable;}


void BoundingView::pokeTransform( const vtkMatrix4x4* vm)
{
    vtkMatrix4x4* m = const_cast<vtkMatrix4x4*>(vm);
    std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ a->PokeMatrix(m);});
}   // end pokeTransform


void BoundingView::updateBounds( const std::vector<cv::Vec6d>& bounds)
{
    size_t n = bounds.size();
    assert(n == _sources.size());
    for ( size_t i = 0; i < n; ++i)
    {
        _sources[i]->SetBounds( &bounds[i][0]);
    }   // end for
}   // end updateBounds


// public
void BoundingView::setVisible( bool visible, ModelViewer* viewer)
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
