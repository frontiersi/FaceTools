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
LandmarkSetView::LandmarkSetView( LandmarkSet::Ptr lset, double r)
    : _lset(lset), _lmrad(r), _viewer(nullptr)
{
    std::for_each( std::begin(lset->ids()), std::end(lset->ids()), [this](int lm){ this->updateLandmark(lm);});
}   // end ctor


// public
LandmarkSetView::~LandmarkSetView()
{
    setVisible( false, nullptr);
    std::for_each( std::begin(_views), std::end(_views), [](auto p){ delete p.second;});
}   // end dtor


// public
void LandmarkSetView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
        std::for_each( std::begin(_views), std::end(_views), [this](auto p){ this->showLandmark( false, p.first);});
    _viewer = viewer;
    if ( _viewer && enable)
        std::for_each( std::begin(_views), std::end(_views), [this](auto p){ this->showLandmark( true, p.first);});
}   // end setVisible


// public
void LandmarkSetView::showLandmark( bool enable, int lm)
{
    if ( _views.count(lm) == 0)
        return;
    _views.at(lm)->setVisible( enable, _viewer);
    _visible.erase(lm);
    if ( enable)
        _visible.insert(lm);
}   // end showLandmark


// public
void LandmarkSetView::highlightLandmark( bool enable, int lm)
{
    if ( _views.count(lm) == 0)
        return;
    enable = enable && _visible.count(lm) > 0;
    _views.at(lm)->setHighlighted( enable);
    _highlighted.erase(lm);
    if ( enable)
        _highlighted.insert(lm);
}   // end highlightLandmark


// public
void LandmarkSetView::setLandmarkRadius( double r)
{
    _lmrad = r;
    std::for_each( std::begin(_views), std::end(_views), [&](auto p){ p.second->setRadius(r);});
}   // end setLandmarkRadius


// public
void LandmarkSetView::pokeTransform( const vtkMatrix4x4* vm)
{ std::for_each( std::begin(_views), std::end(_views), [=](auto p){ p.second->pokeTransform(vm);});}
void LandmarkSetView::fixTransform() { std::for_each( std::begin(_views), std::end(_views), [=](auto p){ p.second->fixTransform();});}
int LandmarkSetView::landmark( const vtkProp* prop) const { return _props.count(prop) > 0 ? _props.at(prop) : -1;}


// public
void LandmarkSetView::updateLandmark( int lm)
{
    assert(lm >= 0);
    if ( !_lset->has(lm))  // Delete landmark from view
    {
        if ( _views.count(lm) > 0)
        {
            showLandmark( false, lm);
            SphereView *sv = _views.at(lm);
            _views.erase(lm);
            _props.erase(sv->prop());
            delete sv;
        }   // end if
    }   // end if
    else    // Landmark added, or an existing landmark's info (position,name) needs updating
    {
        const Landmark* lmk = _lset->get(lm);

        SphereView *sv = nullptr;
        if ( _views.count(lm) > 0) // Get the landmark sphere view to update
            sv = _views.at(lm);
        else    // Landmark was added
        {
            sv = _views[lm] = new SphereView( lmk->pos, _lmrad, true/*pickable*/, true/*fixed scale*/);
            sv->setResolution(20);
            sv->setColour(0.5,0.9,0.0);
            _props[sv->prop()] = lm;
        }   // end if

        sv->setCaption( lmk->name);
        sv->setCentre( lmk->pos);
    }   // end if
}   // end updateLandmark 


// public
void LandmarkSetView::refresh()
{
    std::unordered_set<int> keys; // Copy out the current set of visualised keys
    std::for_each( std::begin(_views), std::end(_views), [&](auto p){ keys.insert(p.first);});
    // Add in the current keys from the landmarks
    std::for_each( std::begin(_lset->ids()), std::end(_lset->ids()), [&](int lm){ keys.insert(lm);});
    // Run update over all
    std::for_each( std::begin(keys), std::end(keys), [this](int lm){ this->updateLandmark(lm);});
}   // end refresh
