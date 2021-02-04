/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Vis/RadialSelectView.h>
using FaceTools::Vis::RadialSelectView;
using FaceTools::Vec3f;


RadialSelectView::RadialSelectView()
    : SimpleView(), _centre( new SphereView( Vec3f(0,0,0), 5, true, true))
{
    _centre->setResolution(40);
    _loop->SetClosed( true);
    _loopActor = addActor(_loop);
    vtkProperty *prop = _loopActor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetLineWidth( 6.0);
}   // end ctor


RadialSelectView::~RadialSelectView()
{
    delete _centre;
}   // end dtor


// Only test centre
bool RadialSelectView::belongs( const vtkProp *prop) const
{
    return _centre->belongs(prop);
}   // end belongs


void RadialSelectView::setVisible( bool v, ModelViewer *mv)
{
    _centre->setVisible( v, mv);
    SimpleView::setVisible( v, mv);
}   // end setVisible


void RadialSelectView::pokeTransform( const vtkMatrix4x4 *m)
{
    _centre->pokeTransform( m);
    SimpleView::pokeTransform( m);
}   // end pokeTransform


void RadialSelectView::setColour( double r, double g, double b, double a)
{
    setActorColour( _loopActor, r, g, b, 0.99);
    _centre->setColour( r, g, b, a);
}   // end setColour


void RadialSelectView::update( const Vec3f &c, const r3d::Mesh &mesh, const std::list<int>& vidxs)
{
    _centre->setCentre( c);
    const size_t N = vidxs.size();
    if ( N > 2)
    {
        _loop->SetNumberOfPoints( N);
        int i = 0;
        for ( int vidx : vidxs)
        {
            const Vec3f& v = mesh.uvtx(vidx);
            _loop->SetPoint( i++, v[0], v[1], v[2]);
        }   // end for
        _loop->Update();
    }   // end if
}   // end update
