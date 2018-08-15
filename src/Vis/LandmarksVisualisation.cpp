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
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool LandmarksVisualisation::apply( const FaceControl* fc, const QPoint*)
{
    if ( _views.count(fc) == 0)
    {
        const FaceModel* fm = fc->data();
        FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
        _views[fc] = new LandmarkSetView( *lmks);
    }   // end if
    return true;
}   // end apply


void LandmarksVisualisation::addActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void LandmarksVisualisation::removeActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


void LandmarksVisualisation::setLandmarkVisible( const FaceModel* fm, int lm, bool v)
{
    const FaceControlSet& fcs = fm->faceControls();
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ assert(_views.count(fc) > 0);});
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){_views.at(fc)->showLandmark(v,lm);});
}   // end setLandmarkVisible


void LandmarksVisualisation::setLandmarkHighlighted( const FaceModel* fm, int lm, bool v)
{
    const FaceControlSet& fcs = fm->faceControls();
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ assert(_views.count(fc) > 0);});
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){_views.at(fc)->highlightLandmark(v,lm);});
}   // end setLandmarkHighlighted


void LandmarksVisualisation::updateLandmark( const FaceModel* fm, int lm)
{
    const FaceControlSet& fcs = fm->faceControls();
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ assert(_views.count(fc) > 0);});
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){_views.at(fc)->updateLandmark(lm);});
}   // end updateLandmark


int LandmarksVisualisation::landmarkProp( const FaceControl* fc, const vtkProp* prop) const
{
    return  _views.count(fc) > 0 ? _views.at(fc)->landmark( prop) : -1;
}   // end landmarkProp


bool LandmarksVisualisation::belongs( const vtkProp* p, const FaceControl* fc) const
{
    bool b = false;
    if (_views.count(fc) > 0)
        b = _views.at(fc)->landmark(p) >= 0;
    return b;
}   // end belongs


// protected
void LandmarksVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->pokeTransform(vm);
}   // end pokeTransform


// protected
void LandmarksVisualisation::fixTransform( const FaceControl* fc)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->fixTransform();
}   // end fixTransform


// protected
void LandmarksVisualisation::purge( const FaceControl* fc)
{
    if ( _views.count(fc) > 0)
    {
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
