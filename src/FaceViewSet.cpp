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

#include <FaceViewSet.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <Vis/FaceView.h>
#include <algorithm>
using FaceTools::FaceViewSet;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FMS;
using FaceTools::FM;


FaceViewSet::Ptr FaceViewSet::create() { return Ptr( new FaceViewSet);}
FaceViewSet::Ptr FaceViewSet::create( const FaceViewSet& fvs) { return Ptr( new FaceViewSet(fvs));}


FaceViewSet::FaceViewSet( const FaceViewSet& fvs)
{
    *this = fvs;
}   // end ctor


FaceViewSet& FaceViewSet::operator=( const FaceViewSet& fvs)
{
    _fvs = fvs._fvs;
    _fmm = fvs._fmm;
    _fms = fvs._fms;
    return *this;
}   // end operator=


FaceViewSet& FaceViewSet::operator+( const FaceViewSet& fvs)
{
    for ( FV* fv : fvs)
        insert(fv);
    return *this;
}   // end operator+


FaceViewSet& FaceViewSet::operator-( const FaceViewSet& fvs)
{
    for ( FV* fv : fvs)
        erase(fv);
    return *this;
}   // end operator-


FaceViewSet& FaceViewSet::operator/( const FaceViewSet& fvs)
{
    for ( FV* fv : _fvs)
    {
        if ( !fvs.has(fv))
            erase(fv);
    }   // end for
    return *this;
}   // end operator/


FaceViewSet FaceViewSet::operator()( const FM* fm) const
{
    FaceViewSet fvs;
    if ( has(fm))
    {
        const std::unordered_set<FV*>& ifvs = _fmm.at(fm);
        std::for_each( std::begin(ifvs), std::end(ifvs), [&](FV* fv){ fvs.insert(fv);});
    }   // end if
    return fvs;
}   // end operator()


size_t FaceViewSet::insert( const FMS& fms)
{
    size_t n = 0;
    for ( const FM* fm : fms)
        n += insert( fm->fvs());
    return n;
}   // end insert


size_t FaceViewSet::insert( const FaceViewSet& fvs)
{
    if ( this == &fvs)  // Can't insert into self!
        return 0;

    size_t cnt = 0;
    for ( FV* fv : fvs)
        if ( insert(fv))
            cnt++;
    return cnt;
}   // end insert


bool FaceViewSet::insert( FV* fv)
{
    bool success = false;
    if ( !has(fv))
    {
        success = true;
        _fvs.insert(fv);
        if ( fv)
        {
            FM* fm = fv->data();
            _fmm[fm].insert(fv);
            _fms.insert(fm);
        }   // end if
    }   // end if
    return success;
}   // end insert


bool FaceViewSet::erase( const FV* cfv)
{
    FV* fv = const_cast<FV*>(cfv);
    size_t a = _fvs.size();
    _fvs.erase(fv);
    if ( fv)
    {
        FM* fm = fv->data();
        _fmm[fm].erase(fv);
        if ( _fmm.at(fm).empty())
        {
            _fmm.erase(fm);
            _fms.erase(fm);
        }   // end if
    }   // end if
    return _fvs.size() < a;
}   // end erase


bool FaceViewSet::insert( const FM* fm)
{
    size_t a = 0;
    for ( FV* fv : fm->fvs())
        a += insert(fv) ? 1 : 0;
    return a > 1;
}   // end insert


bool FaceViewSet::erase( const FM* fm)
{
    size_t a = _fmm.size();
    while ( has(fm))
        erase( *_fmm.at(fm).begin());
    return _fmm.size() < a;
}   // end erase


bool FaceViewSet::has( const FV* fv) const { return _fvs.count(const_cast<FV*>(fv)) == 1;}
bool FaceViewSet::has( const FM* fm) const { return _fmm.count(fm) == 1;}
size_t FaceViewSet::size() const { return _fvs.size();}
bool FaceViewSet::empty() const { return _fvs.empty();}


void FaceViewSet::clear()
{
    _fvs.clear();
    _fmm.clear();
    _fms.clear();
}   // end clear


FV* FaceViewSet::first() const { return empty() ? nullptr : *_fvs.begin();}

const FMS& FaceViewSet::models() const { return _fms;}


FMVS FaceViewSet::viewers() const
{
    FMVS viewers;
    std::for_each( std::begin(_fvs), std::end(_fvs), [&](FV* fv){ viewers.insert(fv->viewer());});
    viewers.erase(nullptr);    // Ensure no null entries (though there shouldn't be any).
    return viewers;
}   // end viewers


FV* FaceViewSet::find( const vtkProp* prop) const
{
    if ( prop)
    {
        for ( FV* fv : _fvs)
        {
            if ( (fv->actor() == prop) || fv->layer( prop))
                return fv;
        }   // end for
    }   // end if
    return nullptr;
}   // end find
