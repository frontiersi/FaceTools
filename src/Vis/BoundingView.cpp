/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <ObjModelBounds.h>  // RFeatures
#include <VtkActorCreator.h> // RVTK
#include <VtkTools.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <algorithm>
using FaceTools::Vis::BoundingView;
using FaceTools::ModelViewer;
using RFeatures::ObjModelBounds;


// public
BoundingView::BoundingView( const ObjModelBounds& bounds, float lw, float r, float g, float b)
    : _viewer(nullptr), _visible(false), _lw(lw)
{
    // Get the untransformed corners to create an upright cuboid
    const cv::Vec6d cb = bounds.cornersAs6d();
    _cubeSource->SetBounds( &cb[0]);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _cubeSource->GetOutputPort());
    _cubeActor->SetMapper(mapper);

    vtkProperty* prop = _cubeActor->GetProperty();

    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetLineWidth( lw);
    prop->SetColor( double(r), double(g), double(b));
    prop->SetOpacity( 0.17);

    // Don't want cuboid actor to be affected by directional lighting
    prop->SetAmbient( 1.0);
    prop->SetDiffuse( 0.0);
    prop->SetSpecular( 0.0);

    setPickable(false);
    pokeTransform( RVTK::toVTK(bounds.transformMatrix()));
}   // end ctor


BoundingView::~BoundingView()
{
    setVisible( false, _viewer);
}   // end dtor


void BoundingView::setPickable( bool v) { _cubeActor->SetPickable(v);}
bool BoundingView::pickable() const { return _cubeActor->GetPickable() != 0;}

void BoundingView::pokeTransform( vtkMatrix4x4* m) { _cubeActor->PokeMatrix( m);}

void BoundingView::setVisible( bool visible, ModelViewer* viewer)
{
    if ( _viewer != viewer && _viewer)
        _viewer->remove(_cubeActor);
    _visible = false;
    _viewer = viewer;
    if ( _viewer)
    {
        if ( visible)
            _viewer->add(_cubeActor);
        else
            _viewer->remove(_cubeActor);
        _visible = visible;
    }   // end if
}   // end setVisible
