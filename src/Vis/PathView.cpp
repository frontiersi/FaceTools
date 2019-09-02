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
#include <FaceTools.h>
#include <VtkActorCreator.h>    // RVTK
#include <VtkTools.h>           // RVTK
#include <vtkProperty.h>
#include <cassert>
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;


PathView::PathView( int id, const std::list<cv::Vec3f>& vtxs)
    : _viewer(nullptr), _id(id), _h0(nullptr), _h1(nullptr), _lprop(nullptr)
#ifndef NDEBUG
      , _pprop(nullptr)
#endif
{
    _h0 = new Handle( 0, _id, vtxs.front(), 1.3);
    _h1 = new Handle( 1, _id, vtxs.back(), 1.3);

    _h0->_sv->setResolution(21);
    _h0->_sv->setColour( 0.1, 0.7, 0.1);    // Green
    _h0->_sv->setOpacity( 0.99);

    _h1->_sv->setResolution(21);
    _h1->_sv->setColour( 0.1, 0.7, 0.1);    // Green
    _h1->_sv->setOpacity( 0.99);

    _h0->_sv->setCaptionColour( Qt::GlobalColor::blue);
    _h1->_sv->setCaptionColour( Qt::GlobalColor::blue);

    update( vtxs);
}   // end ctor


PathView::~PathView()
{
    setVisible( false, nullptr);
    delete _h0;
    delete _h1;
}   // end dtor


void PathView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer)
    {
        _h0->_sv->setVisible( false, _viewer);
        _h1->_sv->setVisible( false, _viewer);
        _viewer->remove(_lprop);
#ifndef NDEBUG
        _viewer->remove(_pprop);
#endif
    }   // end if

    _viewer = viewer;

    if ( enable && _viewer)
    {
        _h0->_sv->setVisible( true, _viewer);
        _h1->_sv->setVisible( true, _viewer);
        _viewer->add(_lprop);
#ifndef NDEBUG
        _viewer->add(_pprop);
#endif
    }   // end if
}   // end setVisible


void PathView::update( const std::list<cv::Vec3f>& vtxs)
{
    _h0->_sv->setCentre( vtxs.front());
    _h1->_sv->setCentre( vtxs.back());

    if ( _lprop)
    {
        if ( _viewer)
            _viewer->remove(_lprop);
        _lprop->Delete();
        _lprop = nullptr;
    }   // end if

#ifndef NDEBUG
    if ( _pprop)
    {
        if ( _viewer)
            _viewer->remove(_pprop);
        _pprop->Delete();
        _pprop = nullptr;
    }   // end if
#endif

    _lprop = RVTK::VtkActorCreator::generateLineActor( vtxs);
    _lprop->SetPickable(false);
    vtkProperty* property = _lprop->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( 3.0);
    property->SetColor( 0.2, 0.4, 0.6);
    property->SetOpacity( 0.99);
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular(0.0);
    if ( _viewer)
        _viewer->add(_lprop);

#ifndef NDEBUG
    _pprop = RVTK::VtkActorCreator::generateLineActor( vtxs);
    _pprop->SetPickable(false);
    property = _pprop->GetProperty();
    property->SetRepresentationToPoints();
    property->SetPointSize(7);
    property->SetColor( 1.0, 0.0, 0.0);
    property->SetOpacity( 0.99);
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular(0.0);
    if ( _viewer)
        _viewer->add(_pprop);
#endif
}   // end update


void PathView::updateColours()
{
    assert(_viewer);
    QColor fg = chooseContrasting(_viewer->backgroundColour());
    _h0->_sv->setCaptionColour( fg);
    _h1->_sv->setCaptionColour( fg);
    //vtkProperty* property = _lprop->GetProperty();
    //property->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end updateColours


// private
PathView::Handle::Handle( int hid, int pid, const cv::Vec3f& c, double r)
    : _hid(hid), _pid(pid), _sv( new SphereView( c, r, true/*pickable*/, true/*fixed scale*/))
{}   // end ctor

// private
PathView::Handle::~Handle() { delete _sv;}
