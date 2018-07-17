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

#include <LandmarksVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


LandmarksVisualisation::LandmarksVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon)
{
}   // end ctor


LandmarksVisualisation::~LandmarksVisualisation()
{
    while (!_lviews.empty())
        purge(_lviews.begin()->first);
}   // end dtor


bool LandmarksVisualisation::isAvailable( const FaceModel* fm) const
{
    return !fm->landmarks()->empty();
}   // end isAvailable


void LandmarksVisualisation::apply( const FaceControl* fc)
{
    if ( _lviews.count(fc) == 0)
    {
        const FaceModel* fm = fc->data();
        FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
        _lviews[fc] = new LandmarkSetView( *lmks);
    }   // end if
}   // end apply


void LandmarksVisualisation::addActors( const FaceControl* fc)
{
    if (_lviews.count(fc) > 0)
        _lviews.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void LandmarksVisualisation::removeActors( const FaceControl* fc)
{
    if (_lviews.count(fc) > 0)
        _lviews.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


const LandmarkSetView* LandmarksVisualisation::landmarks( const FaceControl* fc) const
{
    if ( _lviews.count(fc) == 0)
        return nullptr;
    return _lviews.at(fc);
}   // end landmarks


// public slot
void LandmarksVisualisation::setLandmarkVisible( const FaceControl* fc, int lm, bool v)
{
    if (_lviews.count(fc) > 0)
        _lviews.at(fc)->showLandmark(v,lm);
}   // end setLandmarkVisible


// public slot
void LandmarksVisualisation::setLandmarkHighlighted( const FaceControl* fc, int lm, bool v)
{
    if (_lviews.count(fc) > 0)
        _lviews.at(fc)->highlightLandmark(v,lm);
}   // end setLandmarkHighlighted


// public slot
void LandmarksVisualisation::refreshLandmark( const FaceControl* fc, int lmid)
{
    if (_lviews.count(fc) > 0)
        _lviews.at(fc)->refreshLandmark(lmid);
}   // end refreshLandmark


// public
bool LandmarksVisualisation::belongs( const vtkProp* p, const FaceControl* fc) const
{
    bool b = false;
    if (_lviews.count(fc) > 0)
        b = _lviews.at(fc)->isLandmark(p);
    return b;
}   // end belongs


// protected
void LandmarksVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    if ( _lviews.count(fc) > 0)
        _lviews.at(fc)->pokeTransform(vm);
}   // end pokeTransform


// protected
void LandmarksVisualisation::fixTransform( const FaceControl* fc)
{
    if ( _lviews.count(fc) > 0)
        _lviews.at(fc)->fixTransform();
}   // end fixTransform


// protected
void LandmarksVisualisation::purge( const FaceControl* fc)
{
    if ( _lviews.count(fc) > 0)
    {
        delete _lviews.at(fc);
        _lviews.erase(fc);
    }   // end if
}   // end purge
