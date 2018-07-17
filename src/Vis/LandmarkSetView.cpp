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
#include <Transformer.h>  // RFeatures
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
    : _lset(lset), _lmrad(r), _viewer(nullptr)
{
    std::for_each( std::begin(lset.ids()), std::end(lset.ids()), [this](int lm){ this->refreshLandmark(lm);});
}   // end ctor


// public
LandmarkSetView::~LandmarkSetView()
{
    for ( const auto& lm : _lviews)
    {
        showLandmark( false, lm.first);
        delete lm.second;
    }   // end foreach
}   // end dtor


// public
bool LandmarkSetView::isVisible() const { return !_visible.empty();}


// public
void LandmarkSetView::setVisible( bool enable, ModelViewer* viewer)
{
    while ( isVisible())
        showLandmark( false, *_visible.begin());
    _viewer = viewer;
    if ( enable && _viewer)
        std::for_each( std::begin(_lviews), std::end(_lviews), [this](auto p){ this->showLandmark( true, p.first);});
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
    if ( isLandmarkVisible(lm))
        _visible.insert(lm);

    _highlighted.erase(lm);
    if ( isLandmarkHighlighted(lm))
        _highlighted.insert(lm);
}   // end showLandmark


// public
bool LandmarkSetView::isLandmarkVisible( int lm) const { return _lviews.at(lm)->isVisible();}
bool LandmarkSetView::isLandmarkHighlighted( int lm) const { return _lviews.at(lm)->isHighlighted();}


// public
void LandmarkSetView::highlightLandmark( bool enable, int lm)
{
    if ( _lviews.count(lm) == 0)
        return;

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
    std::for_each( std::begin(_lviews), std::end(_lviews), [&](auto p){ p.second->setRadius(r);});
    _lmrad = r;
}   // end setLandmarkRadius


// public
void LandmarkSetView::pokeTransform( const vtkMatrix4x4* vm)
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](auto p){ p.second->pokeTransform(vm);});
}   // end pokeTransform


// public
void LandmarkSetView::fixTransform()
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](auto p){ p.second->fixTransform();});
}   // end fixTransform


// public
void LandmarkSetView::refreshLandmark( int lm)
{
    assert(lm >= 0);
    if ( !_lset.has(lm))  // Delete landmark from view
    {
        if ( _lviews.count(lm) > 0)
        {
            showLandmark( false, lm);
            SphereView *sv = _lviews.at(lm);
            _lviews.erase(lm);
            _props.erase(sv->prop());
            delete sv;
        }   // end if
    }   // end if
    else    // Landmark added, or an existing landmark's info (position,name) needs updating
    {
        const Landmark* lmk = _lset.get(lm);

        SphereView *sv = nullptr;
        if ( _lviews.count(lm) > 0) // Get the landmark sphere view to update
            sv = _lviews.at(lm);
        else    // Landmark was added
        {
            sv = _lviews[lm] = new SphereView( lmk->pos, _lmrad);
            sv->setResolution(30);
            sv->setColour(1,1,0.2);
            _props[sv->prop()] = lm;
        }   // end if

        sv->setCaption( lmk->name);
        sv->setCentre( lmk->pos);
    }   // end if
}   // end refreshLandmark 


// public
int LandmarkSetView::pointedAt( const QPoint& p) const
{
    int id = -1;
    const vtkProp* prop = _viewer->getPointedAt(p);
    if ( prop && _props.count(prop) > 0)
    {
        id = _props.at(prop);
        if ( _visible.count(id) == 0)   // Ignore if not visible
            id = -1;
    }   // end if
    return id;
}   // end pointedAt


// public
bool LandmarkSetView::isLandmark( const vtkProp* prop) const { return _props.count(prop) > 0;}


// public
const SphereView* LandmarkSetView::landmark( int lmid) const
{
    const SphereView* lv = nullptr;
    if ( _lviews.count(lmid) > 0)
        lv = _lviews.at(lmid);
    return lv;
}   // end landmark
