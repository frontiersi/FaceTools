/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionUpdateThumbnail.h>
#include <FaceModel.h>
#include <cmath>
#include <cassert>
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionUpdateThumbnail::ActionUpdateThumbnail( int w, int h)
    : FaceAction("Thumbnail Updater"), _omv( cv::Size(w,h))
{
    addTriggerEvent( Event::ORIENTATION_CHANGE);
    addTriggerEvent( Event::LANDMARKS_CHANGE);
    addTriggerEvent( Event::GEOMETRY_CHANGE);
}   // end ctor


ActionUpdateThumbnail::~ActionUpdateThumbnail()
{
    while ( !_thumbs.empty())
        _thumbs.erase( _thumbs.begin()->first);
}   // end dtor


const cv::Mat ActionUpdateThumbnail::thumbnail( const FM* fm)
{
    if ( _thumbs.count(fm) > 0)
        return _thumbs.at(fm);
    fm->lockForRead();
    _omv.setModel( fm->model());
    const cv::Vec3f centre = fm->icentre();  // Calculated from landmarks if available
    const RFeatures::Orientation on = fm->orientation();    // Calculated from landmarks if available
    fm->unlock();
    const cv::Vec3f cpos = (500 * on.nvec()) + centre;
    const RFeatures::CameraParams cam( cpos, centre, on.uvec(), 30);
    _omv.setCamera( cam);
    return _thumbs[fm] = _omv.snapshot();
}   // end thumbnail


void ActionUpdateThumbnail::doAction( Event)
{
    const FM* fm = MS::selectedModel();
    _thumbs.erase(fm);
    const cv::Mat img = thumbnail(fm);
    emit updated( fm, img);
}   // end doAction


void ActionUpdateThumbnail::purge( const FM* fm, Event)
{
    _thumbs.erase(fm);
}   // end purge
