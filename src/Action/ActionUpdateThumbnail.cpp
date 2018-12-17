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

#include <ActionUpdateThumbnail.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <cmath>
#include <cassert>
using FaceTools::Action::ActionUpdateThumbnail;
using FaceTools::Action::FaceAction;
using FaceTools::Action::EventSet;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


ActionUpdateThumbnail::ActionUpdateThumbnail( int w, int h)
    : FaceAction(), _omv( cv::Size(w,h), 500)
{
    setRespondToEvent( GEOMETRY_CHANGE);
    setRespondToEvent( ORIENTATION_CHANGE);
}   // end ctor


ActionUpdateThumbnail::~ActionUpdateThumbnail()
{
    while ( !_thumbs.empty())
        purge( _thumbs.begin()->first);
}   // end dtor


const cv::Mat& ActionUpdateThumbnail::thumbnail( const FM* fm)
{
    if ( _thumbs.count(fm) == 0)
    {
        _omv.setModel(fm->info()->cmodel());
        _thumbs[fm] = _omv.snapshot();
    }   // end if
    return _thumbs.at(fm);
}   // end thumbnail


bool ActionUpdateThumbnail::doAction( FVS& fvs, const QPoint&)
{
    const FM* fm = fvs.first()->data();
    purge(fm);
    const cv::Mat& img = thumbnail(fm);
    emit updated( fm, img);
    return true;
}   // end doAction


void ActionUpdateThumbnail::purge( const FM* fm)
{
    if ( _thumbs.count(fm) > 0)
        _thumbs.erase(fm);
}   // end purge
