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

#include <LandmarkSetView.h>
#include <LandmarksManager.h>
#include <FaceTools.h>
#include <algorithm>
#include <iostream>
#include <cassert>
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;
using FaceTools::FaceLateral;
using FaceTools::Landmark::LandmarkSet;
using ViewPair = std::pair<int, SphereView*>;

cv::Vec3d LandmarkSetView::BASE0_COL(0.4, 0.0, 1.0);
cv::Vec3d LandmarkSetView::SPEC0_COL(0.7, 1.0, 0.3);


LandmarkSetView::LandmarkSetView( const LandmarkSet& lmks, double r) : _lmrad(r), _viewer(nullptr)
{
    // set all in lmks
    for ( int id : lmks.ids())
    {
        if ( LDMKS_MAN::landmark(id)->isBilateral())
        {
            set( id, FACE_LATERAL_LEFT, lmks.pos(id, FACE_LATERAL_LEFT));
            set( id, FACE_LATERAL_RIGHT, lmks.pos(id, FACE_LATERAL_RIGHT));
        }   // end if
        else
            set( id, FACE_LATERAL_MEDIAL, lmks.pos(id, FACE_LATERAL_MEDIAL));
    }   // end for
}   // end ctor


LandmarkSetView::~LandmarkSetView()
{
    setVisible( false, nullptr);
    std::for_each( std::begin(_lviews), std::end(_lviews), [](const ViewPair& p){ delete p.second;});
    std::for_each( std::begin(_mviews), std::end(_mviews), [](const ViewPair& p){ delete p.second;});
    std::for_each( std::begin(_rviews), std::end(_rviews), [](const ViewPair& p){ delete p.second;});
}   // end dtor



void LandmarkSetView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
    {
        std::for_each( std::begin(_lviews), std::end(_lviews), [this](const ViewPair& p){ this->showLandmark( false, p.first);});
        std::for_each( std::begin(_mviews), std::end(_mviews), [this](const ViewPair& p){ this->showLandmark( false, p.first);});
        std::for_each( std::begin(_rviews), std::end(_rviews), [this](const ViewPair& p){ this->showLandmark( false, p.first);});
    }   // end if

    _viewer = viewer;

    if ( _viewer && enable)
    {
        std::for_each( std::begin(_lviews), std::end(_lviews), [this](const ViewPair& p){ this->showLandmark( true, p.first);});
        std::for_each( std::begin(_mviews), std::end(_mviews), [this](const ViewPair& p){ this->showLandmark( true, p.first);});
        std::for_each( std::begin(_rviews), std::end(_rviews), [this](const ViewPair& p){ this->showLandmark( true, p.first);});
    }   // end if
}   // end setVisible


void LandmarkSetView::setColour( const cv::Vec3d& c)
{
    const double r = c[0];
    const double g = c[1];
    const double b = c[2];
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](const ViewPair& p){ p.second->setColour( r, g, b);});
    std::for_each( std::begin(_mviews), std::end(_mviews), [=](const ViewPair& p){ p.second->setColour( r, g, b);});
    std::for_each( std::begin(_rviews), std::end(_rviews), [=](const ViewPair& p){ p.second->setColour( r, g, b);});

    assert(_viewer);
    //QColor fg = chooseContrasting( _viewer->backgroundColour());
    const QColor fg = Qt::GlobalColor::blue;
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](const ViewPair& p){ p.second->setCaptionColour( fg);});
    std::for_each( std::begin(_mviews), std::end(_mviews), [=](const ViewPair& p){ p.second->setCaptionColour( fg);});
    std::for_each( std::begin(_rviews), std::end(_rviews), [=](const ViewPair& p){ p.second->setCaptionColour( fg);});
}   // end setColour


void LandmarkSetView::showLandmark( bool enable, int lm)
{
    bool hasLmk = false;

    if ( _lviews.count(lm) > 0)
    {
        _lviews.at(lm)->setVisible( enable, _viewer);
        hasLmk = true;
    }   // end if
    if ( _mviews.count(lm) > 0)
    {
        _mviews.at(lm)->setVisible( enable, _viewer);
        hasLmk = true;
    }   // end if
    if ( _rviews.count(lm) > 0)
    {
        _rviews.at(lm)->setVisible( enable, _viewer);
        hasLmk = true;
    }   // end if

    _visible.erase(lm);
    if ( enable && hasLmk)
        _visible.insert(lm);
}   // end showLandmark


void LandmarkSetView::highlightLandmark( bool enable, int lm, FaceLateral lat)
{
    enable = enable && _visible.count(lm) > 0;

    bool hasLmk = false;

    if ( (lat & FACE_LATERAL_LEFT) && _lviews.count(lm) > 0)
    {
        _lviews.at(lm)->setHighlighted( enable);
        hasLmk = true;
    }   // end if
    if ( (lat & FACE_LATERAL_MEDIAL) && _mviews.count(lm) > 0)
    {
        _mviews.at(lm)->setHighlighted( enable);
        hasLmk = true;
    }   // end if
    if ( (lat & FACE_LATERAL_RIGHT) && _rviews.count(lm) > 0)
    {
        _rviews.at(lm)->setHighlighted( enable);
        hasLmk = true;
    }   // end if

    _highlighted.erase(lm);
    if ( enable && hasLmk)
        _highlighted.insert(lm);
}   // end highlightLandmark


void LandmarkSetView::setLandmarkRadius( double r)
{
    _lmrad = r;
    std::for_each( std::begin(_lviews), std::end(_lviews), [&](const ViewPair& p){ p.second->setRadius(r);});
    std::for_each( std::begin(_mviews), std::end(_mviews), [&](const ViewPair& p){ p.second->setRadius(r);});
    std::for_each( std::begin(_rviews), std::end(_rviews), [&](const ViewPair& p){ p.second->setRadius(r);});
}   // end setLandmarkRadius


void LandmarkSetView::sync( const LandmarkSet& lmks, const cv::Matx44d& d)
{
    for ( auto& p : _lviews)
        p.second->setCentre( RFeatures::transform( d, lmks.pos( p.first, FACE_LATERAL_LEFT)));
    for ( auto& p : _mviews)
        p.second->setCentre( RFeatures::transform( d, lmks.pos( p.first, FACE_LATERAL_MEDIAL)));
    for ( auto& p : _rviews)
        p.second->setCentre( RFeatures::transform( d, lmks.pos( p.first, FACE_LATERAL_RIGHT)));
}   // end sync


int LandmarkSetView::landmarkId( const vtkProp* prop, FaceLateral& lat) const
{
    if ( _lprops.count(prop) > 0)
    {
        lat = FACE_LATERAL_LEFT;
        return _lprops.at(prop);
    }   // end if
    else if ( _mprops.count(prop) > 0)
    {
        lat = FACE_LATERAL_MEDIAL;
        return _mprops.at(prop);
    }   // end else if
    else if ( _rprops.count(prop) > 0)
    {
        lat = FACE_LATERAL_RIGHT;
        return _rprops.at(prop);
    }   // end else if
    return -1;
}   // end landmarkId


void LandmarkSetView::remove( int lm)
{
    assert(lm >= 0);
    _remove( lm, _lviews, _lprops);
    _remove( lm, _mviews, _mprops);
    _remove( lm, _rviews, _rprops);
}   // end remove


void LandmarkSetView::set( int lm, FaceLateral lat, const cv::Vec3f& pos)
{
    if ( lat == FACE_LATERAL_LEFT)
        _set( lm, _lviews, _lprops, pos, " (L)");
    else if ( lat == FACE_LATERAL_MEDIAL)
        _set( lm, _mviews, _mprops, pos);
    else if ( lat == FACE_LATERAL_RIGHT)
        _set( lm, _rviews, _rprops, pos, " (R)");
}   // end set


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


void LandmarkSetView::_set( int lm, SphereMap& views, PropMap& props, const cv::Vec3f& pos, const QString& lats)
{
    SphereView *sv = nullptr;
    if ( views.count(lm) > 0) // Get the landmark sphere view to update
        sv = views.at(lm);
    else    // Landmark was added
    {
        sv = views[lm] = new SphereView( pos, _lmrad, true/*pickable*/, true/*fixed scale*/);
        props[sv->prop()] = lm;
        sv->setColour( BASE0_COL[0], BASE0_COL[1], BASE0_COL[2]);
        sv->setOpacity(0.99);
        sv->setResolution(23);
    }   // end if
    sv->setCaption( (LDMKS_MAN::landmark(lm)->name() + lats).toStdString());
    sv->setCentre( pos);
}   // end _set
