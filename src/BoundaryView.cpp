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

    const bool shown = isVisible();
    setVisible(false, _viewer);    // Ensure old boundary removed from viewer

    const cv::Vec3f fc = FaceTools::calcFaceCentre( _omd);  // Face centre

    const cv::Vec3f& ntip = _omd->getLandmark( FaceTools::Landmarks::NASAL_TIP);
    const int svidx = _omd->getKDTree()->find(ntip);    // Starting vertex for parsing

    const RFeatures::ObjModel::Ptr model = _omd->getObject();
    RFeatures::ObjModelCropper cropper( fc, radius);
    RFeatures::ObjModelTriangleMeshParser parser( model);
    parser.setBoundaryParser( &cropper);

    parser.parse( *model->getFaceIds( svidx).begin());
    const std::list<int>& bverts = cropper.getBoundary();

    std::vector<cv::Vec3f> bvs;
    foreach ( int bv, bverts)
        bvs.push_back( _omd->getObject()->vtx(bv));

    // Create new boundary
    _boundary = RVTK::VtkActorCreator::generateLineActor( bvs, true/*join ends*/);
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
        _boundary->GetProperty()->SetLineWidth( opts.lineWidth);
        const QColor& lcol = _opts.lineColour;
        _boundary->GetProperty()->SetColor( lcol.redF(), lcol.greenF(), lcol.blueF());
    }   // end else
}   // end setOptions
