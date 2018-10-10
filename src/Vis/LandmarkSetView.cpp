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
#include <algorithm>
#include <iostream>
#include <cassert>
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::SphereView;
using ViewPair = std::pair<int, SphereView*>;

LandmarkSetView::LandmarkSetView( double r) : _lmrad(r), _viewer(nullptr) {}


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


void LandmarkSetView::highlightLandmark( bool enable, int lm)
{
    enable = enable && _visible.count(lm) > 0;

    bool hasLmk = false;
    if ( _lviews.count(lm) > 0)
    {
        _lviews.at(lm)->setHighlighted( enable);
        hasLmk = true;
    }   // end if
    if ( _mviews.count(lm) > 0)
    {
        _mviews.at(lm)->setHighlighted( enable);
        hasLmk = true;
    }   // end if
    if ( _rviews.count(lm) > 0)
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


void LandmarkSetView::pokeTransform( const vtkMatrix4x4* vm)
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](const ViewPair& p){ p.second->pokeTransform(vm);});
    std::for_each( std::begin(_mviews), std::end(_mviews), [=](const ViewPair& p){ p.second->pokeTransform(vm);});
    std::for_each( std::begin(_rviews), std::end(_rviews), [=](const ViewPair& p){ p.second->pokeTransform(vm);});
}   // end pokeTransform


void LandmarkSetView::fixTransform()
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [](const ViewPair& p){ p.second->fixTransform();});
    std::for_each( std::begin(_mviews), std::end(_mviews), [](const ViewPair& p){ p.second->fixTransform();});
    std::for_each( std::begin(_rviews), std::end(_rviews), [](const ViewPair& p){ p.second->fixTransform();});
}   // end fixTransform


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


// private
void LandmarkSetView::removeFromView( int lm, SphereMap& views, PropMap& props)
{
    if ( views.count(lm) > 0)
    {
        showLandmark( false, lm);
        SphereView *sv = views.at(lm);
        views.erase(lm);
        props.erase(sv->prop());
        delete sv;
    }   // end if
}   // end removeFromView


// private
void LandmarkSetView::setInView( int lm, SphereMap& views, PropMap& props, const cv::Vec3f& pos)
{
    SphereView *sv = nullptr;
    if ( views.count(lm) > 0) // Get the landmark sphere view to update
        sv = views.at(lm);
    else    // Landmark was added
    {
        sv = views[lm] = new SphereView( pos, _lmrad, true/*pickable*/, true/*fixed scale*/);
        props[sv->prop()] = lm;
    }   // end if
    sv->setResolution(20);
    sv->setColour(0.5,0.9,0.0);
    sv->setCentre( pos);
    sv->setCaption( LDMKS_MAN::landmark(lm)->name().toStdString());
}   // end setInView


void LandmarkSetView::removeFromView( int lm, FaceLateral lat)
{
    assert(lm >= 0);
    if ( lat == FACE_LATERAL_LEFT)
        removeFromView( lm, _lviews, _lprops);
    else if ( lat == FACE_LATERAL_MEDIAL)
        removeFromView( lm, _mviews, _mprops);
    else if ( lat == FACE_LATERAL_RIGHT)
        removeFromView( lm, _rviews, _rprops);
}   // end removeFromView


void LandmarkSetView::setInView( int lm, FaceLateral lat, const cv::Vec3f& pos)
{
    if ( lat == FACE_LATERAL_LEFT)
        setInView( lm, _lviews, _lprops, pos);
    else if ( lat == FACE_LATERAL_MEDIAL)
        setInView( lm, _mviews, _mprops, pos);
    else if ( lat == FACE_LATERAL_RIGHT)
        setInView( lm, _rviews, _rprops, pos);
}   // end setInView
