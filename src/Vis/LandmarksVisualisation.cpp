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

#include <LandmarksVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


LandmarksVisualisation::LandmarksVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon)
{
}   // end ctor


LandmarksVisualisation::~LandmarksVisualisation()
{
    while (!_views.empty())
        purge(const_cast<FV*>(_views.begin()->first));
}   // end dtor


bool LandmarksVisualisation::isAvailable( const FM* fm) const
{
    assert(fm);
    return !fm->landmarks()->empty();
}   // end isAvailable


void LandmarksVisualisation::apply( FV* fv, const QPoint*)
{
    assert(fv);
    if ( !hasView(fv))
        _views[fv] = new LandmarkSetView( fv->data()->landmarks());
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


void LandmarksVisualisation::remove( FV* fv)
{
    assert(fv);
    if ( hasView(fv))
        _views.at(fv)->setVisible( false, fv->viewer());
}   // end remove


void LandmarksVisualisation::setLandmarkVisible( const FM* fm, int lm, bool v)
{
    assert(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](auto fv){ if ( this->hasView(fv)) _views.at(fv)->showLandmark(v,lm);});
}   // end setLandmarkVisible


void LandmarksVisualisation::setLandmarkHighlighted( const FM* fm, int lm, bool v)
{
    assert(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](auto fv){ if ( this->hasView(fv)) _views.at(fv)->highlightLandmark(v,lm);});
}   // end setLandmarkHighlighted


void LandmarksVisualisation::updateLandmark( const FM* fm, int lm)
{
    assert(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](auto fv){ if ( this->hasView(fv)) _views.at(fv)->updateLandmark(lm);});
}   // end updateLandmark


void LandmarksVisualisation::refresh( const FM* fm)
{
    assert(fm);
    const FVS& fvs = fm->fvs();
    std::for_each( std::begin(fvs), std::end(fvs), [=](auto fv){ if ( this->hasView(fv)) _views.at(fv)->refresh();});
}   // end refresh


int LandmarksVisualisation::landmarkProp( const FV* fv, const vtkProp* prop) const
{
    return  hasView(fv) ? _views.at(fv)->landmark( prop) : -1;
}   // end landmarkProp


bool LandmarksVisualisation::belongs( const vtkProp* p, const FV* fv) const
{
    bool b = false;
    if ( hasView(fv))
        b = _views.at(fv)->landmark(p) >= 0;
    return b;
}   // end belongs


// protected
void LandmarksVisualisation::pokeTransform( const FV* fv, const vtkMatrix4x4* vm)
{
    if ( hasView(fv))
        _views.at(fv)->pokeTransform(vm);
}   // end pokeTransform


// protected
void LandmarksVisualisation::fixTransform( const FV* fv)
{
    if ( hasView(fv))
        _views.at(fv)->fixTransform();
}   // end fixTransform


// protected
void LandmarksVisualisation::purge( FV* fv)
{
    if ( hasView(fv))
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge


// protected
bool LandmarksVisualisation::allowShowOnLoad( const FM* fm) const
{
    return !fm->landmarks()->empty();
}   // end allowShowOnLoad


// private
bool LandmarksVisualisation::hasView( const FV* fv) const { return _views.count(fv) > 0;}
