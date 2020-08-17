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
#include <FaceModelViewer.h>
#include <FaceModel.h>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::FV;
using FaceTools::FaceSide;
using FaceTools::Landmark::LandmarkSet;
using FaceTools::Vec3f;
using LMAN = FaceTools::Landmark::LandmarksManager;
using LM = FaceTools::Landmark::Landmark;

namespace {
const Vec3f BASE_COL( 0.6f, 0.2f, 1.0f);    // Not selected
const Vec3f CURR_COL( 0.4f, 1.0f, 0.1f);    // Selected
const Vec3f HGLT_COL( 1.0f, 1.0f, 0.7f);    // Highlighted
const Vec3f MOVG_COL( 1.0f, 0.0f, 0.7f);    // Moving
}   // end namespace


LandmarksVisualisation::LandmarksVisualisation() : _oldfv(nullptr) {}


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
    if ( fv == _oldfv)
        _oldfv = nullptr;
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


void LandmarksVisualisation::refreshState( const FV *fv)
{
    assert( _hasView(fv));
    if ( _oldfv != fv && _oldfv)
        _views.at(_oldfv)->setColour( BASE_COL);

    if ( _oldfv != fv)
    {
        _oldfv = fv;
        const LandmarkSet& lmks = fv->data()->currentLandmarks();
        for ( int id : lmks.ids())
            refreshLandmark( fv, id);
    }   // end if
    _views.at(fv)->setColour( CURR_COL);
}   // end refreshState


void LandmarksVisualisation::setLabelVisible( const FV* fv, int lm, FaceSide lat, bool v)  // Called from handler
{
    assert( LMAN::landmark(lm));
    if ( _hasView(fv))
        _views.at(fv)->setLabelVisible( v, lm, lat);
}   // end setLabelVisible


void LandmarksVisualisation::setLandmarkHighlighted( const FV* fv, int lm, FaceSide lat, bool v)    // Called from handler
{
    assert( LMAN::landmark(lm));
    const Vec3f *col = v ? &HGLT_COL : &CURR_COL;
    if ( !LMAN::isLocked(lm) && v)
        col = &MOVG_COL;
    if ( _hasView(fv))
        _views.at(fv)->setLandmarkColour( *col, lm, lat);
}   // end setLandmarkHighlighted


void LandmarksVisualisation::refreshLandmark( const FV* fv, int id)
{
    assert( _hasView(fv));
    const LandmarkSet& lmks = fv->data()->currentLandmarks();
    const Mat4f &iT = fv->data()->inverseTransformMatrix();
    const LM *lmk = LMAN::landmark(id);
    assert( lmk);

    LandmarkSetView &view = *_views.at(fv);
    view.showLandmark( lmk->isVisible(), id);

    if ( lmk->isBilateral())
    {
        view.set( id, LEFT, r3d::transform( iT, lmks.pos(id, LEFT)));
        view.set( id, RIGHT, r3d::transform( iT, lmks.pos(id, RIGHT)));
    }   // end if
    else
        view.set( id, MID, r3d::transform( iT, lmks.pos(id, MID)));
}   // end refreshLandmark


int LandmarksVisualisation::landmarkId( const FV* fv, const vtkProp* prop, FaceSide &lat) const  // Called from handler
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
