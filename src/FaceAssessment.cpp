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

#include <FaceAssessment.h>
using FaceTools::FaceAssessment;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::PathSet;
using FaceTools::FM;
using FaceTools::Vec3f;
using FaceTools::Mat4f;
using FaceTools::FaceSide;
using FaceTools::Metric::MetricSet;

namespace {
const QString UNKNOWN_NAME = "Unknown";
}   // end namespace


// static
FaceAssessment::Ptr FaceAssessment::create( int id)
{
    return Ptr( new FaceAssessment( id), [](FaceAssessment* x){ delete x;});
}   // end create


FaceAssessment::Ptr FaceAssessment::deepCopy() const
{
    return Ptr( new FaceAssessment(*this), [](FaceAssessment* x){ delete x;});
}   // end deepCopy


// private
FaceAssessment::FaceAssessment( int id) : _id(id), _assessor(UNKNOWN_NAME), _notes("") {}


bool FaceAssessment::setAssessor( const QString &aname)
{
    bool setok = false;
    if ( _assessor != aname)
    {
        _assessor = aname;
        if ( _assessor.isEmpty())
            _assessor = UNKNOWN_NAME;
        setok = true;
    }   // end if
    return setok;
}   // end setAssessor


bool FaceAssessment::setNotes( const QString& notes)
{
    bool setok = false;
    if ( notes != _notes)
    {
        _notes = notes;
        setok = true;
    }   // end if
    return setok;
}   // end setNotes


bool FaceAssessment::setLandmarks( const LandmarkSet &lmks)
{
    bool setok = false;
    if ( !_landmarks.empty() || !lmks.empty())
    {
        _landmarks = lmks;
        setok = true;
    }   // end if
    return setok;
}   // end setLandmarks


void FaceAssessment::transform( const Mat4f &T)
{
    _paths.transform(T);
    _landmarks.transform(T);
}   // end transform


void FaceAssessment::moveToSurface( const FM* fm)
{
    _landmarks.moveToSurface( fm);
    _paths.update(fm);
}   // end moveToSurface


bool FaceAssessment::setPaths( const PathSet &pths)
{
    bool setok = false;
    if ( !_paths.empty() || !pths.empty())
    {
        _paths = pths;
        setok = true;
    }   // end if
    return setok;
}   // end setPaths


MetricSet &FaceAssessment::metrics( FaceSide fs)
{
    MetricSet *mset = &_metrics;
    if ( fs == FaceSide::LEFT)
        mset = &_metricsL;
    else if ( fs == FaceSide::RIGHT)
        mset = &_metricsR;
    return *mset;
}   // end metrics


const MetricSet &FaceAssessment::cmetrics( FaceSide fside) const
{
    const MetricSet *mset = &_metrics;
    if ( fside == FaceSide::LEFT)
        mset = &_metricsL;
    else if ( fside == FaceSide::RIGHT)
        mset = &_metricsR;
    return *mset;
}   // end cmetrics


bool FaceAssessment::hasMetric( int mid) const
{
    return _metrics.ids().count(mid) > 0 || _metricsL.ids().count(mid) > 0 || _metricsR.ids().count(mid) > 0;
}   // end hasMetric


void FaceAssessment::clearMetrics()
{
    _metrics.reset();
    _metricsL.reset();
    _metricsR.reset();
}   // end clearMetrics


bool FaceAssessment::hasContent() const
{
    return (!_assessor.isEmpty() && _assessor != UNKNOWN_NAME) || !_notes.isEmpty()
        || !_landmarks.empty() || !_paths.empty();
}   // end hasContent
