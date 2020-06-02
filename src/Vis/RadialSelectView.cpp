/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
using FaceTools::Vis::FaceView;
using FaceTools::Vec3f;


RadialSelectView::RadialSelectView() : SimpleView()
{
    _centreActor = addActor(_centre);
    _centreActor->SetPickable(true);
    const int rad = 8;
    _centre->SetRadius( rad);
    int res = int(10*rad);
    if ( res % 2 == 0)
        res -= 1;
    _centre->SetPhiResolution(res);
    _centre->SetThetaResolution((res+1)/2);

    _loop->SetClosed( true);
    _loopActor = addActor(_loop);
    vtkProperty *prop = _loopActor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetLineWidth( 8.0);
}   // end ctor


// Only test centre
bool RadialSelectView::belongs( const vtkProp *prop) const { return _centreActor == prop;}


void RadialSelectView::setColour( double r, double g, double b, double a)
{
    vtkProperty *prop = _loopActor->GetProperty();
    prop->SetColor( r, g, b);
    prop->SetOpacity( 0.99);
    prop = _centreActor->GetProperty();
    prop->SetColor( r, g, b);
    prop->SetOpacity( a);
}   // end setColour


void RadialSelectView::update( const Vec3f &upos, const std::vector<const Vec3f*>& vds)
{
    using Vec3d = Eigen::Vector3d;
    Vec3d c = upos.cast<double>();
    _centre->SetCenter( &c[0]);
    _centre->Update();

    if ( vds.size() > 2)
    {
        _loop->SetNumberOfPoints( vds.size());
        for ( size_t i = 0; i < vds.size(); ++i)
        {
            const Vec3f& v = *vds[i];
            _loop->SetPoint( i, v[0], v[1], v[2]);
        }   // end for
        _loop->Update();
    }   // end if
}   // end update
