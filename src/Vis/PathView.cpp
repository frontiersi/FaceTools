/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Vis/PathView.h>
#include <FaceTools.h>
#include <r3dvis/VtkActorCreator.h>
#include <r3dvis/VtkTools.h>
#include <vtkProperty.h>
#include <cassert>
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;
using FaceTools::Vec3f;
using FaceTools::Path;


PathView::PathView( int id)
    : _viewer(nullptr), _isVisible(false), _hasSurface(false),
    _id(id), _h0(nullptr), _h1(nullptr), _g(nullptr),
    _sprop(nullptr), _hprop(nullptr), _dprop(nullptr)
{
    _h0 = new Handle( 0, _id, Vec3f::Zero(), 1.4);
    _h1 = new Handle( 1, _id, Vec3f::Zero(), 1.4);
    _g  = new Handle( 2, _id, Vec3f::Zero(), 1.4);

    _h0->_sv->setResolution(21);
    _h1->_sv->setResolution(21);
    _g->_sv->setResolution(21);

    _h0->_sv->setColour( 0.8, 0.2, 0.0, 0.99);
    _h1->_sv->setColour( 0.8, 0.2, 0.0, 0.99);
    _g->_sv->setColour( 0.6, 0.6, 0.0, 0.99);

    _h0->_sv->setCaptionColour( Qt::white, Qt::black);
    _h1->_sv->setCaptionColour( Qt::white, Qt::black);
    _g->_sv->setCaptionColour( Qt::white, Qt::black);

    _aview.setLineWidth(2.0);
}   // end ctor


PathView::~PathView()
{
    setVisible( false, nullptr);
    delete _h0;
    delete _h1;
    delete _g;
}   // end dtor


void PathView::setVisible( bool enable, ModelViewer *viewer)
{
    _isVisible = false;
    if ( _viewer)
    {
        _h0->_sv->setVisible( false, _viewer);
        _h1->_sv->setVisible( false, _viewer);
        _g->_sv->setVisible( false, _viewer);
        if ( _sprop)
            _removeLineProps();
    }   // end if

    _viewer = viewer;

    if ( enable && _viewer)
    {
        _h0->_sv->setVisible( true, _viewer);
        _h1->_sv->setVisible( true, _viewer);
        _g->_sv->setVisible( _hasSurface, _viewer);
        if ( _sprop)
            _addLineProps();
        _isVisible = true;
    }   // end if
}   // end setVisible


void PathView::_removeLineProps()
{
    _viewer->remove(_sprop);
    _viewer->remove(_hprop);
    _viewer->remove(_dprop);
    _aview.setVisible( false, _viewer);
}   // end _removeLineProps


void PathView::_addLineProps()
{
    _viewer->add(_sprop);
    _viewer->add(_hprop);
    _viewer->add(_dprop);
    _aview.setVisible( _hasSurface, _viewer);
}   // end _addLineProps


namespace {
vtkProperty *setLineActorProperties( vtkSmartPointer<vtkActor> prop, const std::vector<double> &cols, double lw)
{
    prop->SetPickable(false);
    vtkProperty* property = prop->GetProperty();
    property->SetRepresentationToWireframe();
    property->SetRenderLinesAsTubes(false);
    property->SetLineWidth( lw);
    property->SetOpacity( 0.99);
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular(0.0);
    property->SetColor( cols[0], cols[1], cols[2]);
    return property;
}   // end setLineActorProperties
}   // end namespace


void PathView::update( const Path &path)
{
    const Vec3f &h0 = path.handle0();
    const Vec3f &h1 = path.handle1();
    const Vec3f &hd = path.depthHandle();
    const Vec3f &dp = path.depthSurfPoint();
    const Vec3f &dl = path.depthLinePoint();

    _h0->_sv->setCentre( h0);
    _h1->_sv->setCentre( h1);
    _g->_sv->setCentre( hd);

    if ( _sprop && _viewer)
        _removeLineProps();

    // Create the surface path actor
    _sprop = r3dvis::VtkActorCreator::generateLineActor( path.pathVertices(), false);

    // Create the direct line between the handles
    const std::list<Vec3f> hends = { h0, h1};
    _hprop = r3dvis::VtkActorCreator::generateLineActor( hends, false);

    // Create the depth (plumb) line
    const std::list<Vec3f> dends = { dp, dl};
    _dprop = r3dvis::VtkActorCreator::generateLineActor( dends, false);

    std::vector<double> scols = {0.0, 0.2, 0.8};
    std::vector<double> hcols = {0.8, 0.2, 0.0};
    std::vector<double> dcols = {0.5, 0.5, 0.5};
    Vec3f h0d = h0 - hd;
    Vec3f h1d = h1 - hd;
    const bool zeroVecs = h0d.isZero() || h1d.isZero();
    if ( zeroVecs)
        scols = {0.8, 0.2, 0.0};
    setLineActorProperties( _sprop, scols, 5.0);
    setLineActorProperties( _hprop, hcols, 3.0);
    setLineActorProperties( _dprop, dcols, 3.0);

    // Update the angle view
    if ( zeroVecs)
    {
        _aview.setColour( 0.0, 0.7, 0.2, 0.0);  // Fully transparent if no surface distance
        if ( h0 == hd)
            _h0->_sv->setPickable(false);
        else if ( h1 == hd)
            _h1->_sv->setPickable(false);
    }   // end if
    else
    {
        h0d.normalize();
        h1d.normalize();
        Vec3f nrm = h0d.cross(h1d);
        nrm.normalize();
        _aview.update( h0, h1, hd, nrm, path.angle());  // hd was dp
        _aview.setColour( 0.0, 0.7, 0.2, 0.99);
        _h0->_sv->setPickable(true);
        _h1->_sv->setPickable(true);
    }   // end else

    _hasSurface = path.validPath();
    if ( _isVisible && h0 != h1)
    {
        _addLineProps();
        _g->_sv->setVisible( _hasSurface, _viewer);
    }   // end if
}   // end update


void PathView::updateColours()
{
    const QColor bg = _viewer ? _viewer->backgroundColour() : Qt::black;
    const QColor fg = chooseContrasting(bg);
    _h0->_sv->setCaptionColour( fg, bg);
    _h1->_sv->setCaptionColour( fg, bg);
    _g->_sv->setCaptionColour( fg, bg);
    //vtkProperty* property = _sprop->GetProperty();
    //property->SetColor( fg.redF(), fg.greenF(), fg.blueF());
}   // end updateColours


void PathView::setPickable( bool v)
{
    _h0->_sv->setPickable(v);
    _h1->_sv->setPickable(v);
    _g->_sv->setPickable(v);
}   // end setPickable


void PathView::pokeTransform( const vtkMatrix4x4* vm)
{
    vtkMatrix4x4* cvm = const_cast<vtkMatrix4x4*>(vm);
    _sprop->PokeMatrix(cvm);
    _hprop->PokeMatrix(cvm);
    _dprop->PokeMatrix(cvm);
    _h0->_sv->pokeTransform(vm);
    _h1->_sv->pokeTransform(vm);
    _g->_sv->pokeTransform(vm);
    _aview.pokeTransform(vm);
}   // end pokeTransform


// private
PathView::Handle::Handle( int hid, int pid, const Vec3f& c, double r)
    : _hid(hid), _pid(pid), _sv( new SphereView( c, r, true/*pickable*/, true/*fixed scale*/))
{}   // end ctor


// private
PathView::Handle::~Handle() { delete _sv;}


PathView::Handle *PathView::handle( int hid)
{
    assert( hid >= 0 && hid <= 2);
    Handle *h = _g;
    if ( hid == 0)
        h = _h0;
    else if ( hid == 1)
        h = _h1;
    return h;
}   // end handle


Vec3f PathView::Handle::viewPos() const
{
    // View position is the handle centre after applying the view transform
    return r3d::transform( r3dvis::toEigen( _sv->transform()), _sv->centre());
}   // end viewPos

