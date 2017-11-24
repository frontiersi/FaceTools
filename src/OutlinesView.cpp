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

#include <FaceTools.h>                 // FaceTools
#include <OutlinesView.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <cassert>
using FaceTools::OutlinesView;
using FaceTools::ModelViewer;


// public
OutlinesView::OutlinesView( const RFeatures::ObjModel::Ptr model, const ModelViewer::VisOptions* voptions)
    : _voptions( ModelViewer::VisOptions( 1.0,0.3,0.1,1,false,1,5))
{
    if ( voptions)
        _voptions = *voptions;
    _viewer = NULL;
    reset(model);
}   // end ctor


// public
OutlinesView::~OutlinesView()
{
    setVisible( false, NULL);
}   // end setVisible


// public
void OutlinesView::reset( const RFeatures::ObjModel::Ptr model)
{
    const bool showing = isVisible();
    _vloops.clear();
    FaceTools::findBoundaryLoops( model, _vloops);
    setVisible(showing, _viewer);
}   // end reset


// public
void OutlinesView::setVisible( bool visible, ModelViewer* viewer)
{
    if ( _viewer)
    {
        BOOST_FOREACH ( int bprop, _bloops)
            _viewer->remove(bprop);
    }   // end if

    if ( viewer)
    {
        BOOST_FOREACH ( int bprop, _bloops)
            viewer->remove(bprop);
    }   // end if

    _bloops.clear();
    _viewer = viewer;
    if ( visible && viewer)
    {
        BOOST_FOREACH ( const std::vector<cv::Vec3f>& bloop, _vloops)
        {
            const int pid = viewer->addLine( bloop, true, _voptions);
            _bloops.insert(pid);
        }   // end foreach
    }   // end if
}   // end setVisible


// public
bool OutlinesView::isVisible() const { return !_bloops.empty();}
