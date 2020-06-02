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

#include <Vis/LoopView.h>
#include <vtkProperty.h>
using FaceTools::Vis::LoopView;
using FaceTools::Vec3f;


LoopView::LoopView() : SimpleView()
{
    vtkProperty *prop = addActor( _loop)->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    _loop->SetClosed(true);
    _loop->SetPoints( _points);
}   // end ctor


void LoopView::update( const std::vector<const Vec3f*> &vds)
{
    if ( vds.size() > 2)
    {
        _points->SetNumberOfPoints( vds.size());
        for ( size_t i = 0; i < vds.size(); ++i)
            _points->SetPoint( i, &(*vds[i])[0]);
        _points->Modified();
    }   // end if
}   // end update


void LoopView::update( const std::vector<Vec3f> &vds)
{
    if ( vds.size() > 2)
    {
        _points->SetNumberOfPoints( vds.size());
        for ( size_t i = 0; i < vds.size(); ++i)
            _points->SetPoint( i, &vds[i][0]);
        _points->Modified();
    }   // end if
}   // end update
