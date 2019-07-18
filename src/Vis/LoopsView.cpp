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

#include <FaceTools.h>
#include <LoopsView.h>
#include <VtkActorCreator.h>    // RVTK
#include <VtkTools.h>           // RVTK
#include <Transformer.h>        // RFeatures
#include <vtkProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LoopsView;
using FaceTools::ModelViewer;


// public
LoopsView::LoopsView( float lw, double r, double g, double b)
    : _visible(false), _lineWidth(lw), _colour(r,g,b)
{
}   // end ctor


LoopsView::LoopsView( const LoopsView& lv)
{
    *this = lv;
}   // end ctor


LoopsView& LoopsView::operator=( const LoopsView& lv)
{
    _visible = lv._visible;
    _lineWidth = lv._lineWidth;
    _colour = lv._colour;

    for ( const std::list<cv::Vec3f>& line : lv._lines)
        addLoop( line);

    for ( const std::vector<cv::Vec3f>& pts: lv._points)
        addPoints( pts);

    return *this;
}   // end operator=


LoopsView::~LoopsView()
{
    deleteActors();
}   // end dtor


void LoopsView::deleteActors()
{
    std::for_each( std::begin(_actors), std::end(_actors), [](vtkActor *a){ a->Delete();});
    _lines.clear();
    _points.clear();
    _actors.clear();
}   // end deleteActors


void LoopsView::addLoop( const std::list<cv::Vec3f>& line)
{
    vtkActor* actor = RVTK::VtkActorCreator::generateLineActor( line, true);  // Joins ends
    actor->SetPickable(false);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( _lineWidth);
    property->SetColor( _colour[0], _colour[1], _colour[2]);
    property->SetOpacity(0.99);

    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular( 0.0);
  
    _lines.push_back(line); 
    _actors.push_back(actor);
}   // end addLoop


void LoopsView::addPoints( const std::vector<cv::Vec3f>& pts)
{
    vtkActor* actor = RVTK::VtkActorCreator::generatePointsActor( pts);
    actor->SetPickable(false);

    vtkProperty* property = actor->GetProperty();
    property->SetRepresentationToPoints();
    property->SetPointSize( _lineWidth);
    property->SetColor( _colour[0], _colour[1], _colour[2]);
    property->SetOpacity(0.99);

    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular( 0.0);

    _points.push_back(pts);
    _actors.push_back(actor);
}   // end addPoints


void LoopsView::setLineWidth( float lw)
{
    _lineWidth = lw;
}   // end setLineWidth


void LoopsView::changeColour( double r, double g, double b)
{
    for ( vtkActor* a : _actors)
        a->GetProperty()->SetColor( r, g, b);
}   // end changeColour


void LoopsView::setColour( double r, double g, double b)
{
    _colour[0] = r;
    _colour[1] = g;
    _colour[2] = b;
}   // end setColour


void LoopsView::setVisible( bool visible, ModelViewer* viewer)
{
    _visible = false;
    if ( viewer)
    {
        if ( visible)
            std::for_each( std::begin(_actors), std::end(_actors), [=](vtkActor* a){ viewer->add(a);});
        else
            std::for_each( std::begin(_actors), std::end(_actors), [=](vtkActor* a){ viewer->remove(a);});
        _visible = visible;
    }   // end if
}   // end setVisible


void LoopsView::pokeTransform( const vtkMatrix4x4* vm)
{
    vtkMatrix4x4* m = const_cast<vtkMatrix4x4*>(vm);
    std::for_each( std::begin(_actors), std::end(_actors), [=](vtkActor* a){ a->PokeMatrix(m);});

    // Transform the lines and points data
    RFeatures::Transformer tr(RVTK::toCV(m));
    const size_t nlines = _lines.size();
    for ( size_t i = 0; i < nlines; ++i)
        std::for_each( std::begin(_lines[i]), std::end(_lines[i]), [&](cv::Vec3f& v){ tr.transform(v);});
    const size_t npts = _points.size();
    for ( size_t i = 0; i < npts; ++i)
        std::for_each( std::begin(_points[i]), std::end(_points[i]), [&](cv::Vec3f& v){ tr.transform(v);});
}   // end pokeTransform
