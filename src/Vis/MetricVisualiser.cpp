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

#include <Vis/MetricVisualiser.h>
#include <LndMrk/LandmarkSet.h>
#include <Metric/MetricType.h>
#include <FaceModel.h>
using FaceTools::Vis::MetricVisualiser;
using FaceTools::Vis::FV;
using FaceTools::Metric::MetricType;


MetricVisualiser::MetricVisualiser() : _metric(nullptr)
{}   // end ctor


void MetricVisualiser::setMetric( const MetricType *m)
{
    _metric = m;
}   // end setMetric


MetricVisualiser::~MetricVisualiser()
{
    while (!_fvs.empty())
        purge( *_fvs.begin());
}   // end dtor


bool MetricVisualiser::isAvailable( const FV *fv, const QPoint*) const
{
    const FM *fm = fv->data();
    for ( int lmid : metric()->landmarkIds())
        if ( !fm->currentLandmarks().has(lmid))
            return false;
    return true;
}   // end isAvailable


void MetricVisualiser::apply( const FV* fv, const QPoint*)
{
    assert(_fvs.count(fv) == 0);
    doApply(fv);
    _fvs.insert(fv);
}   // end apply


void MetricVisualiser::refresh( const FV* fv)
{
    assert( _fvs.count(fv) > 0);
    doRefresh(fv);
}   // end refresh


void MetricVisualiser::setVisible( FV *fv, bool v)
{
    if ( _fvs.count(fv) > 0)
        doSetVisible( fv, v);
}   // end setVisible


void MetricVisualiser::purge( const FV* fv)
{
    assert(_fvs.count(fv) > 0);
    if (_fvs.count(fv) > 0)
    {
        doPurge(fv);
        _fvs.erase(fv);
    }   // end if
}   // end purge



