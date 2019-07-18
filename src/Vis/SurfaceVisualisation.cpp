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
#include <FaceModel.h>
using FaceTools::Vis::SurfaceMetricsMapper;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;


SurfaceVisualisation::SurfaceVisualisation( SurfaceMetricsMapper::Ptr smm)
    : _smm( smm)
{}   // end ctor


// public
bool SurfaceVisualisation::isAvailable( const FM* fm) const
{
    return _smm->isAvailable(fm);
}   // end isAvailable


void SurfaceVisualisation::apply( FV* fv, const QPoint*)
{
    if ( !_mapped.has(fv))
    {
        if ( _smm->mapMetrics(fv))
            _mapped.insert(fv);
    }   // end if
}   // end apply


bool SurfaceVisualisation::purge( FV* fv, Event e)
{
    if ( _smm->purge( fv->data(), e))
        _mapped.erase(fv);
    return !_mapped.has(fv);
}   // end purge


void SurfaceVisualisation::setVisible( FV* fv, bool v)
{
    assert(fv);
    const SurfaceMetricsMapper* csmm = fv->activeSurface();
    if ( v)
    {
        if ( _mapped.has(fv) && csmm != _smm.get())
            fv->setActiveSurface( _smm.get());
    }   // end if
    else if ( csmm == _smm.get())
        fv->setActiveSurface( nullptr);
}   // end setVisible


// public
bool SurfaceVisualisation::isVisible( const FV *fv) const
{
    return fv->activeSurface() == _smm.get();
}   // end isVisible
