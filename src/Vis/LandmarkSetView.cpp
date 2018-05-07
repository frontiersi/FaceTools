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

#include <LandmarkSetView.h>
#include <VtkTools.h>   // RVTK
#include <ObjModelMover.h>  // RFeatures
#include <ModelViewer.h>
#include <algorithm>
#include <iostream>
#include <cassert>
using FaceTools::Vis::LandmarkSetView;
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;
using FaceTools::LandmarkSet;
using FaceTools::Landmark;

// public
LandmarkSetView::LandmarkSetView( const LandmarkSet& lset, double r)
    : _lset(lset), _lmrad(r), _viewer(NULL)
{
    reset();
}   // end ctor


// public
LandmarkSetView::~LandmarkSetView() { erase();}


// public
void LandmarkSetView::erase()
{
    for ( const auto& lm : _lviews)
    {
        showLandmark( false, lm.first);
        delete lm.second;
    }   // end foreach
    _lviews.clear();
}   // end erase


// public
void LandmarkSetView::reset()
{
    std::cerr << " Resetting LandmarkSetView from data" << std::endl;
    const bool shown = isVisible();
    erase();
    for ( int lm : _lset.ids())
    {
        const Landmark* lmk = _lset.get(lm);
        _lviews[lm] = new SphereView( lmk->pos, _lmrad);
        _lviews[lm]->setCaption( lmk->name);
    }   // end foreach
    setVisible(shown, _viewer);
}   // end reset


// public
bool LandmarkSetView::isVisible() const { return !_visible.empty();}


// public
void LandmarkSetView::setVisible( bool enable, ModelViewer* viewer)
{
    while ( !_visible.empty())
        showLandmark( false, *_visible.begin());

    _viewer = viewer;
    if ( enable && _viewer)
        std::for_each( std::begin(_lviews), std::end(_lviews), [this](const auto& lm){ this->showLandmark( true, lm.first);});
}   // end setVisible


// public
const std::unordered_set<int>& LandmarkSetView::visible() const { return _visible;}
const std::unordered_set<int>& LandmarkSetView::highlighted() const { return _highlighted;}


// public
void LandmarkSetView::showLandmark( bool enable, int lm)
{
    assert( _lviews.count(lm) > 0);
    _lviews.at(lm)->setVisible( enable, _viewer);
    _visible.erase(lm);
    _highlighted.erase(lm);
    if ( isLandmarkVisible(lm))
        _visible.insert(lm);
}   // end showLandmark


// public
bool LandmarkSetView::isLandmarkVisible( int lm) const
{
    return _lviews.at(lm)->isVisible();
}   // end isLandmarkVisible


// public
void LandmarkSetView::highlightLandmark( bool enable, int lm)
{
    assert( _lviews.count(lm) > 0);
    _lviews.at(lm)->highlight( isLandmarkVisible(lm) && enable);
    _highlighted.erase(lm);
    if ( isLandmarkHighlighted(lm))
    {
        _highlighted.insert(lm);
        assert( _visible.count(lm) > 0);
    }   // end if
}   // end highlightLandmark


// public
void LandmarkSetView::setLandmarkRadius( double r)
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [&](const auto& p){ p.second->setRadius(r);});
    _lmrad = r;
}   // end setLandmarkRadius


// public
bool LandmarkSetView::isLandmarkHighlighted( int lm) const
{
    const bool vis = _lviews.at(lm)->isHighlighted();
#ifndef NDEBUG
    if ( vis)
        assert( _highlighted.count(lm) > 0);
#endif
    return vis;
}   // end isLandmarkHighlighted


// public
void LandmarkSetView::transform( const vtkMatrix4x4* vm)
{
    cv::Matx44d m = RVTK::toCV(vm);
    RFeatures::ObjModelMover mover(m);
    std::for_each(std::begin(_lviews), std::end(_lviews), [&](const auto& p)
            {
                cv::Vec3f npos = p.second->centre();
                mover( npos);
                p.second->setCentre(npos);
            });
}   // end transform


// public slot
void LandmarkSetView::refreshLandmark( int lm)
{
    if ( !_lset.has(lm))  // Delete landmark from view
    {
        if ( _lviews.count(lm) > 0)
        {
            showLandmark( false, lm);
            delete _lviews.at(lm);
            _lviews.erase(lm);
        }   // end if
    }   // end if
    else    // Landmark was added, or an existing landmark's position was changed.
    {
        const Landmark* lmk = _lset.get(lm);
        if ( _lviews.count(lm) == 0)    // Landmark was added
            _lviews[lm] = new SphereView( lmk->pos, _lmrad);
        _lviews[lm]->setCaption( lmk->name);
        _lviews[lm]->setCentre( lmk->pos);
    }   // end if
}   // end refreshLandmark 


// public
int LandmarkSetView::pointedAt( const QPoint& p) const
{
    for ( int lm : _visible) // Only check the landmarks that are visible
    {
        if ( _lviews.at(lm)->pointedAt(p))
            return lm;
    }   // end foreach
    return -1;  // Not found
}   // end pointedAt


// public
bool LandmarkSetView::isLandmark( const vtkProp* prop) const
{
    for ( const auto& lm : _lviews)
    {
        if ( lm.second->isProp(prop))
            return true;
    }   // end foreach
    return false;
}   // end isLandmark


// public
const SphereView* LandmarkSetView::landmark( int lmid) const
{
    const SphereView* lv = NULL;
    if ( _lviews.count(lmid) > 0)
        lv = _lviews.at(lmid);
    return lv;
}   // end landmark
