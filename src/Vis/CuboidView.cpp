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

#include <CuboidView.h>
#include <FaceModel.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <vtkCubeSource.h>
#include <vtkPolyDataMapper.h>
#include <algorithm>
using FaceTools::Vis::CuboidView;
using FaceTools::ModelViewer;
using FaceTools::FaceModel;


// public
CuboidView::CuboidView( const FaceModel* fm, float lw, float r, float g, float b)
    : _viewer(NULL), _visible(false), _pickable(false)
{
    for ( const cv::Vec6d& cb : fm->bounds())
    {
        vtkSmartPointer<vtkCubeSource> source = vtkSmartPointer<vtkCubeSource>::New();
        source->SetBounds( &cb[0]);

        vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mapper->SetInputConnection( source->GetOutputPort());
        vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
        actor->SetMapper(mapper);

        actor->GetProperty()->SetRepresentationToWireframe();
        actor->GetProperty()->SetRenderLinesAsTubes(false);
        actor->GetProperty()->SetLineWidth( lw);
        actor->GetProperty()->SetColor( r, g, b);

        // Don't want cuboid actor to be affected by directional lighting
        actor->GetProperty()->SetAmbient( 1.0);
        actor->GetProperty()->SetDiffuse( 0.0);
        actor->GetProperty()->SetSpecular( 0.0);

        _actors.push_back(actor);
    }   // end for
}   // end ctor


CuboidView::~CuboidView()
{
    setVisible( false, _viewer);
}   // end dtor


void CuboidView::setPickable( bool v)
{
    std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ a->SetPickable(v);});
    _pickable = v;
}   // end setPickable


bool CuboidView::pickable() const { return _pickable;}


// public
void CuboidView::setVisible( bool visible, ModelViewer* viewer)
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
