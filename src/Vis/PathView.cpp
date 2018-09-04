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

#include <PathView.h>
#include <VtkActorCreator.h>    // RVTK
#include <VtkTools.h>           // RVTK
#include <vtkProperty.h>
#include <cassert>
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;


// public
PathView::PathView( int id, const std::vector<cv::Vec3f>& vtxs)
    : _viewer(nullptr), _id(id), _h0(nullptr), _h1(nullptr), _lprop(nullptr)
{
    _h0 = new Handle( 0, _id, *vtxs.begin(), 1.4);
    _h1 = new Handle( 1, _id, *vtxs.rbegin(), 1.4);

    _h0->_sv->setResolution(30);
    _h0->_sv->setColour( 1.0, 0.0, 0.0);
    _h0->_sv->setOpacity( 0.4);

    _h1->_sv->setResolution(30);
    _h1->_sv->setColour( 0.0, 0.0, 1.0);
    _h1->_sv->setOpacity( 0.4);

    update( vtxs);
}   // end ctor


// public
PathView::~PathView()
{
    setVisible( false, nullptr);
    delete _h0;
    delete _h1;
}   // end dtor


// public
void PathView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer)
    {
        _h0->_sv->setVisible( false, _viewer);
        _h1->_sv->setVisible( false, _viewer);
        _viewer->remove(_lprop);
    }   // end if

    _viewer = viewer;

    if ( enable && _viewer)
    {
        _h0->_sv->setVisible( true, _viewer);
        _h1->_sv->setVisible( true, _viewer);
        _viewer->add(_lprop);
    }   // end if
}   // end setVisible


// public
void PathView::update( const std::vector<cv::Vec3f>& vtxs)
{
    _h0->_sv->setCentre( *vtxs.begin());
    _h1->_sv->setCentre( *vtxs.rbegin());
    if ( _lprop)
    {
        if ( _viewer)
            _viewer->remove(_lprop);
        _lprop->Delete();
    }   // end if

    _lprop = RVTK::VtkActorCreator::generateLineActor( vtxs);
    _lprop->SetPickable(false);

    vtkProperty* property = _lprop->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( 3.0);
    property->SetColor( 1,1,1);
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular(0.0);
}   // end update


// public
void PathView::pokeTransform( const vtkMatrix4x4* vm)
{
    _h0->_sv->pokeTransform( vm);
    _h1->_sv->pokeTransform( vm);
    _lprop->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));
}   // end transform


void PathView::fixTransform()
{
    _h0->_sv->fixTransform();
    _h1->_sv->fixTransform();
    RVTK::transform( _lprop, _lprop->GetMatrix());
}   // end fixTransform


// private
PathView::Handle::Handle( int hid, int pid, const cv::Vec3f& c, double r)
    : _hid(hid), _pid(pid), _sv( new SphereView( c, r, true/*pickable*/, true/*fixed scale*/))
{}   // end ctor

// private
PathView::Handle::~Handle() { delete _sv;}
