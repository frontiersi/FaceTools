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

#include <SurfaceVisualisation.h>
#include <FaceModelSurfaceData.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <vtkDataSetAttributes.h>
#include <vtkCellData.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
using RFeatures::ObjModelCurvatureMetrics;
using FaceTools::Vis::SurfaceDataMapper;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FaceModelSurfaceData;
using FaceTools::SurfaceData;
using FaceTools::FVS;
using FaceTools::FM;


SurfaceVisualisation::SurfaceVisualisation( SurfaceDataMapper::Ptr sm, const QIcon& icon)
    : BaseVisualisation( sm->label().c_str(), icon), _smapper( sm)
{}   // end ctor


// public
void SurfaceVisualisation::apply( FV* fv, const QPoint*)
{
    if ( !_mapped.has(fv))
    {
        if ( _smapper->mapMetrics(fv))
            _mapped.insert(fv);
    }   // end if

    if ( _mapped.has(fv))
        fv->setActiveSurface( _smapper.get());
}   // end apply


// public
void SurfaceVisualisation::clear( FV* fv)
{
    if ( fv->activeSurface() == _smapper.get())
        fv->setActiveSurface(nullptr);
}   // end clear


// protected
void SurfaceVisualisation::purge( FV* fv)
{
    clear(fv);
    _mapped.erase(fv);
}   // end purge


// protected
void SurfaceVisualisation::purge( const FM* fm)
{
    _smapper->purge(fm);
}   // end purge
