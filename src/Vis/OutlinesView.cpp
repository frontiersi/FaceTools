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
#include <OutlinesView.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::OutlinesView;
using FaceTools::ModelViewer;


// public
OutlinesView::OutlinesView( const RFeatures::ObjModel::Ptr model, float lw, float r, float g, float b)
    : _viewer(NULL), _visible(false)
{
    std::list<std::vector<cv::Vec3f> > loops;
    FaceTools::findBoundaryLoops( model, loops);

    // Create the actors
    for ( const auto& loop : loops)
    {
        vtkSmartPointer<vtkActor> actor = RVTK::VtkActorCreator::generateLineActor( loop, true);  // Joins ends
        actor->GetProperty()->SetRepresentationToWireframe();
        actor->GetProperty()->SetRenderLinesAsTubes(false);
        actor->GetProperty()->SetLineWidth( lw);
        actor->GetProperty()->SetColor( r, g, b);
        _actors.insert(actor);
    }   // end for
}   // end ctor


OutlinesView::~OutlinesView()
{
    setVisible( false, _viewer);
}   // end dtor


// public
void OutlinesView::setVisible( bool visible, ModelViewer* viewer)
{
    if ( _viewer != viewer && _viewer)
    {
        std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->remove(a);});
        _viewer->updateRender();
    }   // end if
    _visible = false;
    _viewer = viewer;
    if ( _viewer)
    {
        if ( visible)
            std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->add(a);});
        else
            std::for_each( std::begin(_actors), std::end(_actors), [=](auto a){ _viewer->remove(a);});
        _visible = visible;
        _viewer->updateRender();
    }   // end if
}   // end setVisible
