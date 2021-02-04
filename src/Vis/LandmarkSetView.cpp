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

#include <Vis/LandmarkSetView.h>
#include <LndMrk/LandmarksManager.h>
#include <FaceTools.h>
#include <algorithm>
#include <iostream>
#include <cassert>
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;
using FaceTools::FaceSide;
using FaceTools::Landmark::LandmarkSet;
using ViewPair = std::pair<int, SphereView*>;
using FaceTools::Vec3f;
using LMAN = FaceTools::Landmark::LandmarksManager;

namespace {
const double ALPHA = 0.99;
const Vec3f BASE_COL( 0.6f, 0.2f, 1.0f);
const Vec3f CURR_COL( 0.4f, 1.0f, 0.1f);
const Vec3f HGLT_COL( 1.0f, 1.0f, 0.7f);
const Vec3f MOVG_COL( 1.0f, 0.0f, 0.7f);
}   // end namespace


LandmarkSetView::LandmarkSetView() : _lmrad(1.0), _viewer(nullptr), _visible(false) {}


LandmarkSetView::~LandmarkSetView()
{
    setVisible( false, nullptr);
    for ( auto& p : _lviews) delete p.second;
    for ( auto& p : _mviews) delete p.second;
    for ( auto& p : _rviews) delete p.second;
}   // end dtor


void LandmarkSetView::set( int lm, FaceSide lat, const Vec3f& pos)
{
    SphereMap *views = nullptr;
    PropMap *props = nullptr;
    if ( lat == LEFT)
    {
        views = &_lviews;
        props = &_lprops;
    }   // end if
    else if ( lat == RIGHT)
    {
        views = &_rviews;
        props = &_rprops;
    }   // end else if
    else
    {
        views = &_mviews;
        props = &_mprops;
    }   // end else

    if ( views->count(lm) == 0)
    {
        SphereView *sv = new SphereView( pos, _lmrad, true/*pickable*/, true/*fixed scale*/);
        sv->setColour( CURR_COL[0], CURR_COL[1], CURR_COL[2], ALPHA);
        sv->setResolution(21);
        QString lmstr = LMAN::makeLandmarkString( lm, lat);
        // Put Inferius/Superius on line beneath to make it easier to see
        lmstr.replace(" Inferius", "\nInferius");
        lmstr.replace(" Superius", "\nSuperius");
        sv->setCaption( lmstr.toStdString());
        (*props)[sv->prop()] = lm;
        (*views)[lm] = sv;
    }   // end if
    views->at(lm)->setCentre( pos);
}   // end set


void LandmarkSetView::setSelectedColour( bool isSelected)
{
    double r = BASE_COL[0];
    double g = BASE_COL[1];
    double b = BASE_COL[2];
    const double a = ALPHA;
    if ( isSelected)
    {
        r = CURR_COL[0];
        g = CURR_COL[1];
        b = CURR_COL[2];
    }   // end if

    for ( auto& p : _lviews) p.second->setColour( r, g, b, a);
    for ( auto& p : _mviews) p.second->setColour( r, g, b, a);
    for ( auto& p : _rviews) p.second->setColour( r, g, b, a);

    if (_viewer)
    {
        const QColor bg = _viewer->backgroundColour();
        const QColor fg = chooseContrasting( bg);
        for ( auto& p : _lviews) p.second->setCaptionColour( fg, bg);
        for ( auto& p : _mviews) p.second->setCaptionColour( fg, bg);
        for ( auto& p : _rviews) p.second->setCaptionColour( fg, bg);
    }   // end if
}   // end setSelectedColour


void LandmarkSetView::setPickable( bool v)
{
    for ( auto& p : _lviews) p.second->setPickable( v);
    for ( auto& p : _mviews) p.second->setPickable( v);
    for ( auto& p : _rviews) p.second->setPickable( v);
}   // end setPickable


void LandmarkSetView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
    {
        for ( auto& p : _lviews) p.second->setVisible( false, _viewer);
        for ( auto& p : _mviews) p.second->setVisible( false, _viewer);
        for ( auto& p : _rviews) p.second->setVisible( false, _viewer);
    }   // end if

    _viewer = viewer;
    _visible = false;

    if ( _viewer && enable)
    {
        for ( auto& p : _lviews) p.second->setVisible( LMAN::landmark(p.first)->isVisible(), _viewer);
        for ( auto& p : _mviews) p.second->setVisible( LMAN::landmark(p.first)->isVisible(), _viewer);
        for ( auto& p : _rviews) p.second->setVisible( LMAN::landmark(p.first)->isVisible(), _viewer);
        _visible = true;
    }   // end if
}   // end setVisible


void LandmarkSetView::showLandmark( bool enable, int lm)
{
    enable = enable && _visible && LMAN::landmark(lm)->isVisible();
    if ( _lviews.count(lm) > 0)
    {
        assert( _rviews.count(lm) > 0);
        assert( _viewer);
        _lviews.at(lm)->setVisible( enable, _viewer);
        _rviews.at(lm)->setVisible( enable, _viewer);
    }   // end if
    else if ( _mviews.count(lm) > 0)
    {
        assert( _viewer);
        _mviews.at(lm)->setVisible( enable, _viewer);
    }   // end else if
}   // end showLandmark


void LandmarkSetView::setLabelVisible( bool enable, int lm, FaceSide lat)
{
    enable = enable && LMAN::landmark(lm)->isVisible();
    if ( (lat & LEFT) && _lviews.count(lm) > 0)
        _lviews.at(lm)->showCaption( enable);
    else if ( (lat & MID) && _mviews.count(lm) > 0)
        _mviews.at(lm)->showCaption( enable);
    else if ( (lat & RIGHT) && _rviews.count(lm) > 0)
        _rviews.at(lm)->showCaption( enable);
}   // end setLabelVisible


void LandmarkSetView::_setLandmarkColour( const Vec3f &col, int lm, FaceSide lat)
{
    const double r = col[0];
    const double g = col[1];
    const double b = col[2];
    const double a = ALPHA;

    if ( (lat & LEFT) && _lviews.count(lm) > 0)
        _lviews.at(lm)->setColour( r, g, b, a);
    else if ( (lat & MID) && _mviews.count(lm) > 0)
        _mviews.at(lm)->setColour( r, g, b, a);
    else if ( (lat & RIGHT) && _rviews.count(lm) > 0)
        _rviews.at(lm)->setColour( r, g, b, a);
}   // end _setLandmarkColour


void LandmarkSetView::setHighlighted( bool enable, int lm, FaceSide lat)
{
    const Vec3f *col = &CURR_COL;
    if ( enable)
        col = LMAN::isLocked(lm) ? &HGLT_COL : &MOVG_COL;
    _setLandmarkColour( *col, lm, lat);
}   // end setHighlighted


void LandmarkSetView::pokeTransform( const vtkMatrix4x4* vd)
{
    for ( auto& p : _lviews) p.second->pokeTransform( vd);
    for ( auto& p : _mviews) p.second->pokeTransform( vd);
    for ( auto& p : _rviews) p.second->pokeTransform( vd);
}   // end pokeTransform


int LandmarkSetView::landmarkId( const vtkProp* prop, FaceSide& lat) const
{
    if ( _lprops.count(prop) > 0)
    {
        lat = LEFT;
        return _lprops.at(prop);
    }   // end if

    if ( _mprops.count(prop) > 0)
    {
        lat = MID;
        return _mprops.at(prop);
    }   // end if

    if ( _rprops.count(prop) > 0)
    {
        lat = RIGHT;
        return _rprops.at(prop);
    }   // end if

    return -1;
}   // end landmarkId


void LandmarkSetView::remove( int lm)
{
    assert(lm >= 0);
    _remove( lm, _lviews, _lprops);
    _remove( lm, _mviews, _mprops);
    _remove( lm, _rviews, _rprops);
}   // end remove


void LandmarkSetView::_remove( int lm, SphereMap& views, PropMap& props)
{
    if ( views.count(lm) > 0)
    {
        showLandmark( false, lm);
        SphereView *sv = views.at(lm);
        views.erase(lm);
        props.erase(sv->prop());
        delete sv;
    }   // end if
}   // end _remove
