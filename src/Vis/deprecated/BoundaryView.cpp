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

#include <SphericalBoundaryView.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
using FaceTools::Vis::SphericalBoundaryView;
using FaceTools::ModelViewer;
using RFeatures::ObjModelRegionSelector;


// public
SphericalBoundaryView::SphericalBoundaryView( const cv::Vec3f& v, double r)
    : _isshown(false), _viewer(NULL)
{
    // Create the blob
    _csource = vtkSmartPointer<vtkSphereSource>::New();
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _csource->GetOutputPort())
    _cblob = vtkSmartPointer<vtkActor>::New();
    _cblob->SetMapper(mapper);

    _cblob->GetProperty()->SetColour(0.4f, 0.2f, 0.8f);
    _cblob->GetProperty()->SetOpacity( 0.3);
    setCentre(v);
    setRadius(r);
}   // end ctor


// public
SphericalBoundaryView::~SphericalBoundaryView() { setVisible(false,NULL);}


// public
void SphericalBoundaryView::setCentre( const cv::Vec3f& v)
{
    _csource->SetCenter(v[0], v[1], v[2]);
    _csource->Update();
}   // end setCentre


// public
void SphericalBoundaryView::centre() const { return _csource->GetCenter();}


// public
void SphericalBoundaryView::setRadius( double r)
{
    _csource->SetRadius(r);
    _csource->Update();
}   // end setRadius


// public
double SphericalBoundaryView::radius() const { return _csource->GetRadius();}

/*
// public
void SphericalBoundaryView::reset()
{
    const bool shown = isVisible();
    erase();

    const IntSet *bverts = _rselector->getBoundary();
    // Cannot get boundary since boundary loop doesn't exist.
    if ( !bverts || bverts->size() < 3)
        return;

    // Create line pairs
    const RFeatures::ObjModel::Ptr model = _rselector->getObject();
    std::vector<cv::Vec3f> bpairs;
    for ( int bv : *bverts)
    {
        const IntSet& cvs = model->getConnectedVertices(bv);
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

    _cblob->

    setVisible(shown, _viewer);
}   // end reset
*/


// public
void SphericalBoundaryView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer && _boundary)
        _viewer->remove(_cblob);

    _isshown = false;
    _viewer = viewer;
    if ( enable && _boundary && _viewer)
    {
        _viewer->add(_cblob);
        _isshown = true;
    }   // end if
}   // end setVisible


// public
void SphericalBoundaryView::transform( const vtkMatrix4x4* m)
{
    RVTK::transform( _cblob, m);
}   // end transform
