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
#include <cassert>
#include <iostream>
using FaceTools::LandmarkGroupView;
using FaceTools::LandmarkView;
using FaceTools::VisualisationOptions;
using FaceTools::ModelViewer;


// public
LandmarkGroupView::LandmarkGroupView( ModelViewer* viewer, const FaceTools::ObjMetaData::Ptr objmeta)
    : _viewer(viewer), _objmeta(objmeta)
{
    reset();
}   // end ctor


// public
LandmarkGroupView::~LandmarkGroupView()
{
    erase();
}   // end dtor


// public
void LandmarkGroupView::showAll( bool enable)
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
        showLandmark( lm.first, enable);
}   // end showAll


// public
bool LandmarkGroupView::isShown() const
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        if ( !isShown( lm.first))
            return false;
    }   // end foreach
    return true;
}   // end isShown


// public
void LandmarkGroupView::showLandmark( const std::string& lm, bool enable)
{
    assert( _lviews.count(lm) > 0);
    _lviews.at(lm)->show(enable);
}   // end showLandmark


// public
bool LandmarkGroupView::isShown( const std::string& lm) const
{
    assert( _lviews.count(lm) > 0);
    return _lviews.at(lm)->isShown();
}   // end isShown


// public
void LandmarkGroupView::highlightLandmark( const std::string& lm, bool enable)
{
    assert( _lviews.count(lm) > 0);
    _lviews.at(lm)->highlight(enable);
}   // end highlightLandmark


// public
void LandmarkGroupView::setVisualisationOptions( const VisualisationOptions::Landmarks& visopts)
{
    _visopts = visopts;
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
        lm.second->setVisualisationOptions(visopts);
}   // end setVisualisationOptions


// public
void LandmarkGroupView::erase()
{
    typedef std::pair<std::string, LandmarkView*> LMPair;
    foreach ( const LMPair& lm, _lviews)
    {
        lm.second->show(false); // Ensure landmark not present anymore
        delete lm.second;
    }   // end foreach
    _lviews.clear();
}   // end erase


// public
void LandmarkGroupView::reset()
{
    erase();
    boost::unordered_set<std::string> lmnames;
    _objmeta->getLandmarks( lmnames);
    foreach ( const std::string& lm, lmnames)
        _lviews[lm] = new LandmarkView( _viewer, _objmeta->getLandmarkMeta(lm), _visopts);
}   // end reset


// public slot
void LandmarkGroupView::updateLandmark( const std::string& lm, const cv::Vec3f* pos)
{
    if ( pos != NULL)   // Landmark was added, or an existing landmark's position was changed.
    {
        assert( _objmeta->hasLandmark(lm));
        if ( _lviews.count(lm) == 0)    // Landmark was added
            _lviews[lm] = new LandmarkView( _viewer, _objmeta->getLandmarkMeta(lm), _visopts);
        else
            _lviews[lm]->update();
        showLandmark(lm,true);  // Ensure moved or added landmark is seen next render
    }   // end if
    else if ( !_objmeta->hasLandmark(lm))    // Landmark was deleted
    {
        showLandmark(lm,false); // Remove erased landmark from viewer
        delete _lviews.at(lm);
        _lviews.erase(lm);
    }   // end else if
    else
    {
        std::cerr << "Invalid logic in LandmarkGroupView::updateLandmark!" << std::endl;
        assert(false);
    }   // end else
}   // end updateLandmark


// public slot
void LandmarkGroupView::selectLandmark( const std::string& lm, bool enable)
{
    _lviews.at(lm)->highlight(enable);
}   // end selectLandmark


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
