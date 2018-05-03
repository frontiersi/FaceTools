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
#include <FaceModel.h>
#include <FaceModelViewer.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
using FaceTools::Vis::BoundaryView;
using FaceTools::FaceControl;


// public
BoundaryView::BoundaryView( const FaceControl& fcont) : _fcont(fcont), _isshown(false)
{}   // end ctor


// public
BoundaryView::~BoundaryView() { setVisible(false);}


// public
void BoundaryView::setVisible( bool enable)
{
    FaceTools::ModelViewer* viewer = _fcont.viewer();
    if ( _boundary)
        viewer->remove(_boundary);

    _isshown = false;
    if ( enable && _boundary)
    {
        viewer->add(_boundary);
        _isshown = true;
    }   // end if
}   // end setVisible


// public
bool BoundaryView::setBoundary( const std::unordered_set<int>* bverts)
{
    // Cannot get boundary since boundary loop doesn't exist.
    if ( !bverts || bverts->size() < 3)
        return false;

    const bool shown = isVisible();
    setVisible(false);    // Ensure old boundary removed from viewer

    // Create line pairs
    const RFeatures::ObjModel::Ptr model = _fcont.data()->model();
    std::vector<cv::Vec3f> bpairs;
    for ( int bv : *bverts)
    {
        const std::unordered_set<int>& cvs = model->getConnectedVertices(bv);
        for ( int cv : cvs)
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

    setVisible( shown); // Restore visibility state
    return true;
}   // end setBoundary
