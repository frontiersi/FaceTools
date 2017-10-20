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
BoundaryView::BoundaryView( ModelViewer* viewer, const ObjMetaData::Ptr omd)
    : _viewer(viewer), _omd(omd), _faceCropFactor(1.0), _isshown(false)
{
    assert( omd->hasLandmark( FaceTools::Landmarks::NASAL_TIP));
}   // end ctor


void BoundaryView::show( bool enable)
{
    if ( enable)
    {
        if ( !_isshown)
            _viewer->add(_boundary);
        _isshown = true;
    }   // end if
    else
    {
        if ( _isshown)
            _viewer->remove(_boundary);
        _isshown = false;
    }   // end else
}   // end show


bool BoundaryView::isShown() const { return _isshown;}


// public
void BoundaryView::reset()
{
    // Calc boundary for given radius
    const double radius = FaceTools::calcFaceCropRadius( _omd, _faceCropFactor);
    if ( radius <= 0)
        return;

    const bool shown = isShown();
    show(false);    // Ensure old boundary removed from viewer

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
    _boundary->GetProperty()->SetRenderPointsAsSpheres(true);
    _boundary->GetProperty()->SetRenderLinesAsTubes(true);
    // Set actor visualisation options
    setVisualisationOptions( _visopts);

    show( shown);   // Restore shown state with new boundary
}   // end reset


// public
void BoundaryView::setFaceCropFactor( double G)
{
    _faceCropFactor = G;
}   // end setFaceCropFactor


// public
void BoundaryView::setVisualisationOptions( const VisualisationOptions::Boundary& visopts)
{
    _visopts = visopts;
    _boundary->GetProperty()->SetLineWidth( _visopts.lineWidth);
    _boundary->GetProperty()->SetPointSize( _visopts.vertexSize);
    const QColor& lcol = _visopts.lineColour;
    _boundary->GetProperty()->SetColor( lcol.redF(), lcol.greenF(), lcol.blueF());
}   // end setVisualisationOptions
