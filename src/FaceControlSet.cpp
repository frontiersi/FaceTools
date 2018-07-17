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
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::FaceModelSet;
using FaceTools::FaceViewerSet;

// public
FaceControlSet::FaceControlSet( const FaceControlSet& fcs)
{
    *this = fcs;
}   // end ctor


// public
FaceControlSet& FaceControlSet::operator=( const FaceControlSet& fcs)
{
    _fcs = fcs._fcs;
    _fmm = fcs._fmm;
    _fms = fcs._fms;
    return *this;
}   // end operator=


// public
FaceControlSet& FaceControlSet::operator+( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : fcs)
        insert(fc);
    return *this;
}   // end operator+


// public
FaceControlSet& FaceControlSet::operator-( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : fcs)
        erase(fc);
    return *this;
}   // end operator-


// public
FaceControlSet& FaceControlSet::operator/( const FaceControlSet& fcs)
{
    for ( FaceControl* fc : _fcs)
    {
        if ( !fcs.has(fc))
            erase(fc);
    }   // end for
    return *this;
}   // end operator/


// public
FaceControlSet FaceControlSet::operator()( const FaceModel* fm) const
{
    FaceControlSet fcs;
    if ( has(fm))
    {
        const std::unordered_set<FaceControl*>& ifcs = _fmm.at(fm);
        std::for_each( std::begin(ifcs), std::end(ifcs), [&](auto fc){ fcs.insert(fc);});
    }   // end if
    return fcs;
}   // end operator()


// public
bool FaceControlSet::insert( FaceControl* fc)
{
    bool success = false;
    if ( !has(fc))
    {
        success = true;
        _fcs.insert(fc);
        if ( fc)
        {
            _fmm[fc->data()].insert(fc);
            _fms.insert(fc->data());
        }   // end if
    }   // end if
    return success;
}   // end insert


// public
bool FaceControlSet::erase( FaceControl* fc)
{
    size_t a = _fcs.size();
    _fcs.erase(fc);
    if ( fc)
    {
        FaceModel* fm = fc->data();
        _fmm[fm].erase(fc);
        if ( _fmm.at(fm).empty())
        {
            _fmm.erase(fm);
            _fms.erase(fm);
        }   // end if
    }   // end if
    return _fcs.size() < a;
}   // end erase


// public
bool FaceControlSet::erase( const FaceModel* fm)
{
    size_t a = _fmm.size();
    while ( has(fm))
        erase( *_fmm.at(fm).begin());
    return _fmm.size() < a;
}   // end erase


// public
bool FaceControlSet::has( FaceControl* fc) const { return _fcs.count(fc) == 1;}
bool FaceControlSet::has( const FaceModel* fm) const { return _fmm.count(fm) == 1;}
size_t FaceControlSet::size( const FaceModel* fm) const { return has(fm) ? _fmm.at(fm).size() : 0;}
size_t FaceControlSet::size() const { return _fcs.size();}
bool FaceControlSet::empty() const { return _fcs.empty();}


// public
void FaceControlSet::clear()
{
    _fcs.clear();
    _fmm.clear();
    _fms.clear();
}   // end clear


// public
FaceControl* FaceControlSet::first() const
{
    FaceControl* fc = nullptr;
    if ( !empty())
        fc = *_fcs.begin();
    return fc;
}   // end first


// public
const FaceModelSet& FaceControlSet::models() const { return _fms;}


// public
FaceViewerSet FaceControlSet::viewers() const
{
    FaceViewerSet viewers;
    std::for_each( std::begin(_fms), std::end(_fms), [&](auto fm){
                for ( FaceControl* fc : fm->faceControls()) // For all FaceControl's that map to the model
                    viewers.insert(fc->viewer());
            });
    viewers.erase(nullptr);    // Ensure no null entries (though there shouldn't be any).
    return viewers;
}   // end viewers


// public
FaceViewerSet FaceControlSet::directViewers() const
{
    FaceViewerSet viewers;
    std::for_each( std::begin(_fcs), std::end(_fcs), [&](auto fc){ viewers.insert(fc->viewer());});
    viewers.erase(nullptr);    // Ensure no null entries (though there shouldn't be any).
    return viewers;
}   // end directViewers


// public
FaceControl* FaceControlSet::find( const vtkProp* prop) const
{
    if ( !prop)
        return nullptr;
    for ( FaceControl* fc : _fcs)
    {
        if ( fc->view()->belongs( prop))
            return fc;
    }   // end for
    return nullptr;
}   // end find

