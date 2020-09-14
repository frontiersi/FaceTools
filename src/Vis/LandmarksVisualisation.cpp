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

#include <Vis/LandmarksVisualisation.h>
#include <LndMrk/LandmarksManager.h>
#include <Action/ModelSelector.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::FV;
using FaceTools::FaceSide;
using FaceTools::Landmark::LandmarkSet;
using LMAN = FaceTools::Landmark::LandmarksManager;
using MS = FaceTools::Action::ModelSelector;


LandmarksVisualisation::~LandmarksVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool LandmarksVisualisation::isAvailable( const FV *fv, const QPoint*) const
{
    return fv->data()->hasLandmarks();
}   // end isAvailable


void LandmarksVisualisation::apply( const FV* fv, const QPoint*)
{
    assert( !_hasView(fv));
    _views[fv] = new LandmarkSetView;
}   // end apply


void LandmarksVisualisation::purge( const FV* fv)
{
    assert( _hasView(fv));
    delete _views.at(fv);
    _views.erase(fv);
}   // end purge


void LandmarksVisualisation::setVisible( FV* fv, bool v)
{
    assert( _hasView(fv));
    _views.at(fv)->setVisible( v, fv->viewer());
}   // end setVisible


bool LandmarksVisualisation::isVisible( const FV* fv) const
{
    return _hasView(fv) && _views.at(fv)->isVisible();
}   // end isVisible


void LandmarksVisualisation::syncWithViewTransform( const FV* fv)
{
    assert( _hasView(fv));
    _views.at(fv)->pokeTransform(fv->transformMatrix());
}   // end syncWithViewTransform


void LandmarksVisualisation::refresh( const FV *fv)
{
    assert( _hasView(fv));
    const LandmarkSet& lmks = fv->data()->currentLandmarks();
    for ( int lm : lmks.ids())
    {
        if ( LMAN::isBilateral(lm))
        {
            refreshLandmarkPosition( fv, lm, LEFT);
            refreshLandmarkPosition( fv, lm, RIGHT);
        }   // end if
        else
            refreshLandmarkPosition( fv, lm, MID);
    }   // end for
    _views.at(fv)->setSelectedColour( fv == MS::selectedView());
}   // end refresh


// Called from handler
void LandmarksVisualisation::setLabelVisible( const FV* fv, int lm, FaceSide lat, bool v)
{
    if ( _hasView(fv))
        _views.at(fv)->setLabelVisible( v, lm, lat);
}   // end setLabelVisible


// Called from handler
void LandmarksVisualisation::setLandmarkHighlighted( const FV* fv, int lm, FaceSide lat, bool v)
{
    if ( _hasView(fv))
        _views.at(fv)->setHighlighted( v, lm, lat);
}   // end setLandmarkHighlighted


void LandmarksVisualisation::refreshLandmarkPosition( const FV* fv, int lm, FaceSide lat)
{
    assert( _hasView(fv));
    const LandmarkSet& lmks = fv->data()->currentLandmarks();
    _views.at(fv)->set( lm, lat, r3d::transform( fv->data()->inverseTransformMatrix(), lmks.pos(lm, lat)));
}   // end refreshLandmarkPosition

  
// Called from handler
int LandmarksVisualisation::landmarkId( const FV* fv, const vtkProp* prop, FaceSide &lat) const
{
    return _hasView(fv) ? _views.at(fv)->landmarkId( prop, lat) : -1;
}   // end landmarkId


bool LandmarksVisualisation::belongs( const vtkProp* p, const FV* fv) const
{
    bool b = false;
    if ( _hasView(fv))
    {
        FaceSide ignored;
        b = _views.at(fv)->landmarkId(p, ignored) >= 0;
    }   // end if
    return b;
}   // end belongs


bool LandmarksVisualisation::_hasView( const FV* fv) const { return fv && _views.count(fv) > 0;}
