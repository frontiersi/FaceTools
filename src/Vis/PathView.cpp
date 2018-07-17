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

#include <PathView.h>
#include <VtkTools.h>       // RVTK
#include <vtkProperty.h>
#include <cassert>
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;
using FaceTools::Path;


// public
PathView::PathView( const Path& path)
    : _viewer(nullptr), _path(path), _h0(nullptr), _h1(nullptr), _linePropID(-1)
{
    _h0 = new Handle(this, *path.vtxs.begin(), 1.4);
    _h1 = new Handle(this, *path.vtxs.rbegin(), 1.4);

    _h0->_sv->setResolution(30);
    _h0->_sv->setColour( 1.0, 0.0, 0.0);
    _h0->_sv->setOpacity( 0.4);

    _h1->_sv->setResolution(30);
    _h1->_sv->setColour( 0.0, 0.0, 1.0);
    _h1->_sv->setOpacity( 0.4);
}   // end ctor


// public
PathView::~PathView()
{
    removePath();
    delete _h0;
    delete _h1;
}   // end dtor


// public
void PathView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer)
        removePath();
    _viewer = viewer;
    if ( enable && _viewer)
        addPath();
}   // end setVisible


// public
void PathView::update()
{
    setVisible( true, _viewer);
}   // end update


// public
void PathView::pokeTransform( const vtkMatrix4x4* vm)
{
    assert(_viewer);
    _h0->_sv->pokeTransform( vm);
    _h1->_sv->pokeTransform( vm);
    // Transform every vertex in the line actor
    assert(_linePropID >= 0);
    vtkActor* lp = static_cast<vtkActor*>(_viewer->getProp(_linePropID));
    assert(lp);
    lp->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));
}   // end transform


void PathView::fixTransform()
{
    assert(_viewer);
    _h0->_sv->fixTransform();
    _h1->_sv->fixTransform();
    vtkActor* lp = static_cast<vtkActor*>(_viewer->getProp(_linePropID));
    assert(lp);
    RVTK::transform( lp, lp->GetMatrix());
}   // end fixTransform


// private
void PathView::removePath()
{
    assert(_viewer);
    _h0->_sv->setVisible( false, _viewer);
    _h1->_sv->setVisible( false, _viewer);
    if ( _linePropID >= 0)
        _viewer->remove(_linePropID);
    _linePropID = -1;
}   // end removePath


// private
void PathView::addPath()
{
    assert(_viewer);
    const ModelViewer::VisOptions loptions( 1.0f,1.0f,1.0f,1.0f,false,1.0f, /*Line Width*/2.2f/**/);
    _linePropID = _viewer->addLine( _path.vtxs, false, loptions);
    vtkActor* lactor = static_cast<vtkActor*>( _viewer->getProp(_linePropID));
    lactor->SetPickable(false); // The measurement line shouldn't be pickable!
    vtkProperty* property = lactor->GetProperty();
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular(0.0);

    _h0->_sv->setCentre( *_path.vtxs.begin());
    _h0->_sv->setVisible( true, _viewer);

    _h1->_sv->setCentre( *_path.vtxs.rbegin());
    _h1->_sv->setVisible( true, _viewer);
}   // end addPath


// private
PathView::Handle::Handle( PathView* pv, const cv::Vec3f& c, double r)
    : _host(pv), _sv( new SphereView( c, r))
{}   // end ctor


// private
PathView::Handle::~Handle()
{
    delete _sv;
}   // end dtor
