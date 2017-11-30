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
#include <FaceTools.h>
#include <Landmarks.h>
#include <ObjModelTools.h>
using FaceTools::BoundaryView;
using FaceTools::ModelViewer;
using FaceTools::ObjMetaData;
#include <vtkProperty.h>
#include <sstream>
#include <cassert>


// public
BoundaryView::BoundaryView( const ObjMetaData::Ptr omd)
    : _viewer(NULL), _omd(omd), _isshown(false)
{
    _opts.cropFactor = 2.0;
    reset();
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
void BoundaryView::reset()
{
    // Calc boundary for given radius
    const double radius = FaceTools::calcFaceCropRadius( _omd, _opts.cropFactor);
    if ( radius <= 0)   // If can't calculate, the boundary can't be set
        return;

    assert( _omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP));
    assert( _omd->getKDTree() != NULL);

    const cv::Vec3f fc = FaceTools::calcFaceCentre( _omd);  // Face centre
    const int svidx = _omd->getKDTree()->find( _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP));
    const RFeatures::ObjModel::Ptr model = _omd->getObject();
    RFeatures::ObjModelCropper::Ptr cropper = RFeatures::ObjModelCropper::create( model, fc, svidx);
    cropper->adjustRadius( radius);

    // Cannot get boundary since boundary loop doesn't exist.
    const IntSet* bverts = cropper->getBoundary();
    if ( bverts->size() < 3)
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

    setOptions( _opts); // Set actor visualisation options
    setVisible( shown, _viewer);   // Restore shown state with new boundary
}   // end reset


// public
void BoundaryView::setOptions( const FaceTools::ModelOptions::Boundary& opts)
{
    double oldRad = _opts.cropFactor;
    _opts = opts;
    if ( !_boundary)
        return;

    if ( _opts.cropFactor != oldRad)
        reset();    // Will recurse back into this function with same crop factor
    else
    {
        _boundary->GetProperty()->SetLineWidth( 3.0);
        _boundary->GetProperty()->SetColor( 0.4f, 0.2f, 0.8f);
    }   // end else
}   // end setOptions
