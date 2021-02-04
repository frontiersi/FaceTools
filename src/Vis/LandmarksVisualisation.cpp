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

#include <Vis/LandmarksVisualisation.h>
#include <LndMrk/LandmarksManager.h>
#include <ModelSelect.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FaceSide;
using FaceTools::Landmark::LandmarkSet;
using LMAN = FaceTools::Landmark::LandmarksManager;
using MS = FaceTools::ModelSelect;


bool LandmarksVisualisation::isAvailable( const FV *fv) const
{
    return fv->data()->hasLandmarks();
}   // end isAvailable


void LandmarksVisualisation::purge( const FV* fv) { _views.erase(fv);}


void LandmarksVisualisation::setVisible( FV* fv, bool v)
{
    if ( _hasView(fv))
    {
        _views.at(fv).setVisible( v, fv->viewer());
        _views.at(fv).setSelectedColour( MS::selectedModel() == fv->data());
    }   // end if
}   // end setVisible


bool LandmarksVisualisation::isVisible( const FV* fv) const
{
    return _hasView(fv) && _views.at(fv).isVisible();
}   // end isVisible


void LandmarksVisualisation::syncTransform( const FV* fv)
{
    if ( _hasView(fv))
        _views.at(fv).pokeTransform(fv->transformMatrix());
}   // end syncTransform


void LandmarksVisualisation::refresh( FV *fv)
{
    LandmarkSetView &lsv = _views[fv];  // Creates if doesn't exist
    const Mat4f imat = fv->data()->inverseTransformMatrix();
    const LandmarkSet& lmks = fv->data()->currentLandmarks();

    for ( int lm : lmks.ids())
    {
        if ( LMAN::isBilateral(lm))
        {
            lsv.set( lm, LEFT, r3d::transform( imat, lmks.pos(lm, LEFT)));
            lsv.set( lm, RIGHT, r3d::transform( imat, lmks.pos(lm, RIGHT)));
        }   // end if
        else
            lsv.set( lm, MID, r3d::transform( imat, lmks.pos(lm, MID)));
    }   // end for
}   // end refresh


void LandmarksVisualisation::setSelectedColour( const FV *fv, bool v)
{
    if ( _hasView(fv))
        _views.at(fv).setSelectedColour( v);
}   // end setSelectedColour


void LandmarksVisualisation::setPickable( const FV *fv, bool v)
{
    if ( _hasView(fv))
        _views.at(fv).setPickable( v);
}   // end setPickable


void LandmarksVisualisation::refreshLandmarkPosition( const FV* fv, int lm, FaceSide lat)
{
    if ( _hasView(fv))
    {
        const FM *fm = fv->data();
        const LandmarkSet& lmks = fm->currentLandmarks();
        _views.at(fv).set( lm, lat, r3d::transform( fm->inverseTransformMatrix(), lmks.pos(lm, lat)));
    }   // end if
}   // end refreshLandmarkPosition


// Called from handler
void LandmarksVisualisation::setLabelVisible( const FV* fv, int lm, FaceSide lat, bool v)
{
    if ( _hasView(fv))
        _views.at(fv).setLabelVisible( v, lm, lat);
}   // end setLabelVisible


// Called from handler
void LandmarksVisualisation::setLandmarkHighlighted( const FV* fv, int lm, FaceSide lat, bool v)
{
    if ( _hasView(fv))
        _views.at(fv).setHighlighted( v, lm, lat);
}   // end setLandmarkHighlighted

  
// Called from handler
int LandmarksVisualisation::landmarkId( const FV* fv, const vtkProp* prop, FaceSide &lat) const
{
    return _hasView(fv) ? _views.at(fv).landmarkId( prop, lat) : -1;
}   // end landmarkId


bool LandmarksVisualisation::belongs( const vtkProp* p, const FV* fv) const
{
    bool b = false;
    if ( _hasView(fv))
    {
        FaceSide ignored;
        b = _views.at(fv).landmarkId(p, ignored) >= 0;
    }   // end if
    return b;
}   // end belongs


bool LandmarksVisualisation::_hasView( const FV* fv) const { return fv && _views.count(fv) > 0;}
