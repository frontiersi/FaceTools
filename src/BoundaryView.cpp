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

#include <BoundaryView.h>
#include <VtkActorCreator.h>    // RVTK
using FaceTools::BoundaryView;
using FaceTools::ModelViewer;
#include <vtkProperty.h>
#include <sstream>
#include <cassert>


// public
BoundaryView::BoundaryView() : _viewer(NULL), _isshown(false)
{
}   // end ctor


// public
BoundaryView::~BoundaryView()
{
    setVisible(false, NULL);
}   // en dtor


// public
void BoundaryView::setVisible( bool enable, ModelViewer* viewer)
{
#ifndef NDEBUG
    if ( enable)
        assert(_boundary);
#endif
    if ( _viewer)
        _viewer->remove(_boundary);

    if ( viewer)
        viewer->remove(_boundary);

    _isshown = false;
    _viewer = viewer;
    if ( enable && viewer && _boundary != NULL)
    {
        viewer->add(_boundary);
        _isshown = true;
    }   // end if
}   // end setVisible


// public
bool BoundaryView::isVisible() const { return _isshown;}


// public
void BoundaryView::setBoundary( const RFeatures::ObjModel::Ptr model, const IntSet* bverts)
{
    // Cannot get boundary since boundary loop doesn't exist.
    if ( !bverts || bverts->size() < 3)
        return;

    const bool shown = isVisible();
    setVisible(false, _viewer);    // Ensure old boundary removed from viewer

    // Create line pairs
    std::vector<cv::Vec3f> bpairs;
    foreach ( int bv, *bverts)
    {
        const IntSet& cvs = model->getConnectedVertices(bv);
        foreach ( int cv, cvs)
        {
            if ( bverts->count(cv) > 0)
            {
                bpairs.push_back( model->vtx( bv));
                bpairs.push_back( model->vtx( cv));
            }   // end if
        }   // end foreach
    }   // end foreach

    // Create new boundary
    _boundary = RVTK::VtkActorCreator::generateLinePairsActor( bpairs);
    _boundary->GetProperty()->SetRepresentationToWireframe();
    _boundary->GetProperty()->SetLineWidth( 3.0);
    _boundary->GetProperty()->SetColor( 0.4f, 0.2f, 0.8f);

    setVisible( shown, _viewer);   // Restore shown state with new boundary
}   // end setBoundary
