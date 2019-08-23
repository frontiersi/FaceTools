/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <cassert>
using FaceTools::FaceAssessment;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::PathSet;
using FaceTools::FM;


// static
FaceAssessment::Ptr FaceAssessment::create( int id, const QString& aname)
{
    return Ptr( new FaceAssessment( id, aname), [](FaceAssessment* x){ delete x;});
}   // end create


FaceAssessment::Ptr FaceAssessment::createClone( int id, const QString& aname)
{
    FaceAssessment* fa = new FaceAssessment( *this); // Deep copy landmarks, paths, metrics
    fa->_id = id;
    fa->setAssessor(aname);
    fa->_notes = "";
    return Ptr( fa, [](FaceAssessment* x){ delete x;});
}   // end createClone


FaceAssessment::Ptr FaceAssessment::deepCopy() const
{
    return Ptr( new FaceAssessment(*this), [](FaceAssessment* x){ delete x;});
}   // end deepCopy


bool FaceAssessment::setAssessor( const QString &aname)
{
    bool setok = false;
    if ( _assessor != aname)
    {
        _assessor = aname;
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


bool FaceAssessment::setLandmarks( const LandmarkSet::Ptr lmks)
{
    bool setok = false;
    if ( !_landmarks->empty() || !lmks->empty())
    {
        _landmarks = lmks;
        setok = true;
    }   // end if
    return setok;
}   // end setLandmarks


void FaceAssessment::setLandmarkPosition( int lid, FaceTools::FaceLateral flat, const cv::Vec3f& pos)
{
    const bool okay = _landmarks->set( lid, pos, flat); // Adds if not already present
    assert(okay);
}   // end setLandmarkPosition


bool FaceAssessment::swapLandmarkLaterals()
{
    bool setok = false;
    if ( !_landmarks->empty())
    {
        _landmarks->swapLaterals();
        setok = true;
    }   // end if
    return setok;
}   // end swapLandmarkLaterals


bool FaceAssessment::addTransform( const cv::Matx44d &T)
{
    bool setok = false;
    if ( !_landmarks->empty())
    {
        _landmarks->addTransformMatrix(T);
        setok = true;
    }   // end if

    if ( !_paths->empty())
    {
        _paths->transform(T);
        setok = true;
    }   // end if

    return setok;
}   // end addTransform


void FaceAssessment::fixTransform()
{
    _landmarks->fixTransformMatrix();
    // Note here that path vertices are never passed through a transform.
}   // end fixTransform


bool FaceAssessment::moveLandmarksToSurface( const FM* fm)
{
    bool setok = false;
    if ( !_landmarks->empty())
    {
        _landmarks->moveToSurface( fm);
        setok = true;
    }   // end if
    return setok;
}   // end moveLandmarksToSurface


bool FaceAssessment::setPaths( PathSet::Ptr pths)
{
    bool setok = false;
    if ( !_paths->empty() || !pths->empty())
    {
        _paths = pths;
        setok = true;
    }   // end if
    return setok;
}   // end setPaths


int FaceAssessment::addPath( const cv::Vec3f& pos) { return _paths->addPath(pos);}

bool FaceAssessment::removePath( int pid) { return _paths->has(pid) ? _paths->removePath(pid) : false;}
bool FaceAssessment::renamePath( int pid, const QString &nm) { return _paths->has(pid) ? _paths->renamePath(pid, nm.toStdString()) : false;}

bool FaceAssessment::setPathPosition( const FM *fm, int pid, int h, const cv::Vec3f& pos)
{
    Path* path = _paths->path(pid);
    if ( !path)
        return false;
    if ( h == 0)
        *path->vtxs.begin() = pos;
    else
        *path->vtxs.rbegin() = pos;
    path->recalculate( fm);
    return true;
}   // end setPathPosition


void FaceAssessment::recalculatePaths(const FM *fm) { _paths->recalculate(fm);}


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
    return !_assessor.isEmpty() || !_notes.isEmpty()
        || !_landmarks->empty() || !_paths->empty();
}   // end hasContent


// private
FaceAssessment::FaceAssessment( int id, const QString& aname)
    : _id(id), _assessor(aname), _notes(""),
      _landmarks( LandmarkSet::create()), _paths(PathSet::create())
{
}   // end ctor


FaceAssessment::~FaceAssessment(){}


// private
FaceAssessment::FaceAssessment( const FaceAssessment& fa) { *this = fa;}

// private
FaceAssessment& FaceAssessment::operator=( const FaceAssessment &fa)
{
    _id = fa._id;
    _assessor = fa._assessor;
    _notes = fa._notes;
    _landmarks = fa._landmarks->deepCopy();
    _paths = fa._paths->deepCopy();
    _metrics = fa._metrics;
    _metricsL = fa._metricsL;
    _metricsR = fa._metricsR;
    return *this;
}   // end operator=
