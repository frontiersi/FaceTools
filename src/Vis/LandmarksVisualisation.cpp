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

#include <LandmarksVisualisation.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <ChangeEvents.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LandmarksVisualisation;
using FaceTools::Vis::LandmarkSetView;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using FaceTools::Action::ActionVisualise;

namespace {
void updateRender( const std::unordered_map<const FaceControl*, LandmarkSetView*>& lviews)
{
    std::unordered_set<ModelViewer*> viewers;
    std::for_each( std::begin(lviews), std::end(lviews), [&](auto f){ viewers.insert(f.first->viewer());});
    std::for_each( std::begin(viewers), std::end(viewers), [](auto v){ v->updateRender();});
}   // end updateRender
}   // end namespace


LandmarksVisualisation::LandmarksVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : BaseVisualisation(dname, icon, keys)
{
}   // end ctor


LandmarksVisualisation::~LandmarksVisualisation()
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [](auto f){ delete f.second;});
    _lviews.clear();
}   // end dtor


bool LandmarksVisualisation::isAvailable( const FaceModel* fm) const
{
    return !fm->landmarks().empty();
}   // end isAvailable


void LandmarksVisualisation::apply( const FaceControl* fc)
{
    if ( _lviews.count(fc) == 0)
        _lviews[fc] = new LandmarkSetView( fc->data()->landmarks());
}   // end apply


void LandmarksVisualisation::addActors( const FaceControl* fc)
{
    _lviews.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void LandmarksVisualisation::removeActors( const FaceControl* fc)
{
    _lviews.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


const LandmarkSetView* LandmarksVisualisation::landmarks( const FaceControl* fc) const
{
    if ( !fc || _lviews.count(fc) == 0)
        return NULL;
    return _lviews.at(fc);
}   // end landmarks


// public slot
void LandmarksVisualisation::setLandmarkVisible( int lm, bool v)
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](auto f){ f.second->showLandmark(v,lm);});
    updateRender(_lviews);
}   // end setLandmarkVisible


// public slot
void LandmarksVisualisation::setLandmarkHighlighted( int lm, bool v)
{
    std::for_each( std::begin(_lviews), std::end(_lviews), [=](auto f){ f.second->highlightLandmark(v,lm);});
    updateRender(_lviews);
}   // end setLandmarkHighlighted


// public slot
void LandmarksVisualisation::refreshLandmark( const FaceControl* fc, int lmid)
{
    assert(_lviews.count(fc) > 0);
    _lviews.at(fc)->refreshLandmark(lmid);
}   // end refreshLandmark


// protected
void LandmarksVisualisation::setAction( ActionVisualise* av)
{
    using namespace FaceTools::Action;
    av->addRespondTo(LANDMARK_ADDED);
    av->addRespondTo(LANDMARK_DELETED);
    av->addRespondTo(LANDMARK_CHANGED);
}   // end setAction


// protected
void LandmarksVisualisation::respondTo( const FaceControl* fc)
{
    assert(_lviews.count(fc) > 0);
    _lviews.at(fc)->reset();
}   // end respondTo


// protected
bool LandmarksVisualisation::belongs( const vtkProp* p, const FaceControl* fc) const
{
    assert(_lviews.count(fc) > 0);
    return _lviews.at(fc)->isLandmark(p);
}   // end belongs


// protected
void LandmarksVisualisation::transform( const FaceControl* fc, const vtkMatrix4x4* m)
{
    assert(_lviews.count(fc) > 0);
    _lviews.at(fc)->transform(m);
}   // end transform


// protected
void LandmarksVisualisation::burn( const FaceControl* fc)
{
    if (_lviews.count(fc) > 0)
    {
        delete _lviews.at(fc);
        _lviews.erase(fc);
    }   // end if
}   // end burn
