/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#include <FaceControlSet.h>
#include <FaceControl.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::FaceModelSet;
using FaceTools::FaceViewerSet;

// public
FaceControlSet::FaceControlSet( const FaceControlSet& fcs)
{
    _fcs = fcs._fcs;
}   // end ctor


// public
FaceControlSet& FaceControlSet::operator=( const FaceControlSet& fcs)
{
    _fcs = fcs._fcs;
    return *this;
}   // end operator=


// public
FaceControlSet& FaceControlSet::operator+( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : fcs)
        _fcs.insert(fc);
    return *this;
}   // end operator+


// public
FaceControlSet& FaceControlSet::operator-( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : fcs)
        _fcs.erase(fc);
    return *this;
}   // end operator-


// public
FaceControlSet& FaceControlSet::operator/( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : _fcs)
    {
        if ( !fcs.has(fc))
            _fcs.erase(fc);
    }   // end for
    return *this;
}   // end operator/


// public
bool FaceControlSet::insert( FaceControl* fc)
{
    assert(fc);
    bool success = false;
    if ( count(fc) == 0)
    {
        success = true;
        _fcs.insert(fc);
    }   // end if
    return success;
}   // end insert


// public
bool FaceControlSet::erase( FaceControl* fc)
{
    size_t a = _fcs.size();
    _fcs.erase(fc);
    return _fcs.size() < a;
}   // end erase


// public
size_t FaceControlSet::count( FaceControl* fc) const { return _fcs.count(fc);}
bool FaceControlSet::has( FaceControl* fc) const { return count(fc) == 1;}
size_t FaceControlSet::size() const { return _fcs.size();}
bool FaceControlSet::empty() const { return _fcs.empty();}
void FaceControlSet::clear() { _fcs.clear();}


// public
FaceControl* FaceControlSet::first() const
{
    FaceControl* fc = NULL;
    if ( !empty())
        fc = *_fcs.begin();
    return fc;
}   // end first


// public
FaceControl* FaceControlSet::find( const vtkProp* prop) const
{
    if ( !prop)
        return NULL;
    for ( FaceControl* fc : _fcs)
    {
        if ( fc->view()->belongs( prop))
            return fc;
    }   // end for
    return NULL;
}   // end find


// public
FaceModelSet FaceControlSet::models() const
{
    FaceModelSet models;
    std::for_each( std::begin(_fcs), std::end(_fcs), [&](auto fc){ models.insert(fc->data());});
    return models;
}   // end models


// public
FaceViewerSet FaceControlSet::viewers() const
{
    FaceViewerSet viewers;
    std::for_each( std::begin(_fcs), std::end(_fcs), [&](auto fc){ viewers.insert(fc->viewer());});
    viewers.erase(NULL);    // Ensure no NULL entries!
    return viewers;
}   // end viewers
