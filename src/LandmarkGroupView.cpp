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

#include <LandmarkGroupView.h>
#include <LandmarkView.h>
#include <ModelViewer.h>
#include <iostream>
#include <cassert>
using FaceTools::ModelOptions;
using FaceTools::LandmarkGroupView;
using FaceTools::LandmarkView;
using FaceTools::ModelViewer;
using FaceTools::ObjMetaData;


// public
LandmarkGroupView::LandmarkGroupView( const ObjMetaData::Ptr omd)
    : _viewer(NULL), _omd(omd)
{
    reset();
}   // end ctor


// public
LandmarkGroupView::~LandmarkGroupView()
{
    erase();
}   // end dtor


// public
void LandmarkGroupView::setVisible( bool enable, ModelViewer* viewer)
{
    typedef std::pair<std::string, LandmarkView*> LMPair;

    if ( _viewer)
    {
        foreach ( const LMPair& lm, _lviews)
            lm.second->setVisible( false, _viewer);
    }   // end if

    if ( viewer)
    {
        foreach ( const LMPair& lm, _lviews)
            lm.second->setVisible( false, viewer);
    }   // end if

    _viewer = viewer;
    if ( enable && viewer)
    {
        foreach ( const LMPair& lm, _lviews)
            lm.second->setVisible( true, viewer);
    }   // end if
}   // end setVisible


// public
bool LandmarkGroupView::isVisible() const
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        if ( isLandmarkVisible( lm.first))
            return true;
    }   // end foreach
    return false;
}   // end isVisible


// public
void LandmarkGroupView::getVisibleLandmarks( std::vector<std::string>& lmks) const
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        if ( isLandmarkVisible( lm.first))
            lmks.push_back(lm.first);
    }   // end foreach
}   // end getVisibleLandmarks


// public
void LandmarkGroupView::showLandmark( bool enable, const std::string& lm)
{
    assert( _lviews.count(lm) > 0);
    _lviews.at(lm)->setVisible( enable, _viewer);
}   // end showLandmark


// public
bool LandmarkGroupView::isLandmarkVisible( const std::string& lm) const
{
    assert( _lviews.count(lm) > 0);
    return _lviews.at(lm)->isVisible();
}   // end isLandmarkVisible


// public
void LandmarkGroupView::highlightLandmark( bool enable, const std::string& lm)
{
    if ( _lviews.count(lm) > 0)
        _lviews.at(lm)->highlight( _lviews.at(lm)->isVisible() && enable);
}   // end highlightLandmark


// public
void LandmarkGroupView::setOptions( const ModelOptions& opts)
{
    _opts = opts;
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
        lm.second->setOptions(opts);
}   // end setOptions


// public
void LandmarkGroupView::erase()
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        lm.second->setVisible( false, _viewer); // Ensure landmark not present anymore
        delete lm.second;
    }   // end foreach
    _lviews.clear();
}   // end erase


// public
void LandmarkGroupView::reset()
{
    const bool shown = isVisible();
    erase();
    boost::unordered_set<std::string> lmnames;
    _omd->getLandmarks( lmnames);
    foreach ( const std::string& lm, lmnames)
    {
        const FaceTools::Landmarks::Landmark* lmk = _omd->getLandmarkMeta(lm);
        _lviews[lm] = new LandmarkView( _opts);
        _lviews[lm]->set( lm, lmk->pos);
    }   // end foreach
    setVisible(shown, _viewer);
}   // end reset


// public
void LandmarkGroupView::updateLandmark( const std::string& lm, const cv::Vec3f* pos)
{
    if ( pos != NULL)   // Landmark was added, or an existing landmark's position was changed.
    {
        assert( _omd->hasLandmark(lm));
        if ( _lviews.count(lm) == 0)    // Landmark was added
            _lviews[lm] = new LandmarkView( _opts);
        _lviews[lm]->set( lm, *pos);
    }   // end if
    else if ( !_omd->hasLandmark(lm))    // Landmark was deleted
    {
        delete _lviews.at(lm);
        _lviews.erase(lm);
    }   // end else if
}   // end updateLandmark


// public
std::string LandmarkGroupView::pointedAt( const QPoint& p) const
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        if ( lm.second->isPointedAt(p))
            return lm.first;
    }   // end foreach
    return "";  // Not found
}   // end pointedAt


// public
bool LandmarkGroupView::isLandmark( const vtkProp* prop) const
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        if ( lm.second->isProp(prop))
            return true;
    }   // end foreach
    return false;
}   // end isLandmark
