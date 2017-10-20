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

#include <SurfacePathDrawer.h>
#include <ModelViewer.h>
#include <VtkTools.h>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
using FaceTools::SurfacePathDrawer;
using FaceTools::ObjMetaData;


SurfacePathDrawer::SurfacePathDrawer( FaceTools::ModelViewer* mv, const std::string& munits)
    : _viewer(mv),
      _munits(munits),
      _lenText( vtkSmartPointer<vtkTextActor>::New()),
      _lenCaption( vtkSmartPointer<vtkCaptionActor2D>::New())
{
    assert(mv != NULL);
    _lenText->GetTextProperty()->SetJustificationToRight();
    _lenText->GetTextProperty()->SetFontFamilyToArial();
    _lenText->GetTextProperty()->SetFontSize(16);
    _lenText->GetTextProperty()->SetColor( 1, 1, 1);

    _lenCaption->BorderOff();
    _lenCaption->GetCaptionTextProperty()->BoldOff();
    _lenCaption->GetCaptionTextProperty()->ItalicOff();
    _lenCaption->GetCaptionTextProperty()->ShadowOff();
    _lenCaption->GetCaptionTextProperty()->SetFontFamilyToArial();
    _lenCaption->GetCaptionTextProperty()->SetFontSize(8);

    _pathStart = QPoint(-1,-1);
    _path = RVTK::ModelPathDrawer::create( _viewer->getRenderWindow()->GetInteractor());
    _path->setLineWidth(2);
    _path->setLineColour(1.0, 1.0, 0.3);
    _path->setPointColour(1.0,0.0,0.0);
    _path->setPointSize(9);
}   // end ctor


void SurfacePathDrawer::setUnits( const std::string& units) { _munits = units;}
bool SurfacePathDrawer::isShown() const { return _path && _path->getVisibility();}


// public
void SurfacePathDrawer::show( bool enable)
{
    _path->setVisibility(enable);
    if (enable)
    {
        _viewer->add(_lenText);
        _viewer->add(_lenCaption);
    }   // end if
    else
    {
        _viewer->remove(_lenText);
        _viewer->remove(_lenCaption);
    }   // end else
    _viewer->updateRender();
}   // end show


// public
void SurfacePathDrawer::setActor( const vtkSmartPointer<vtkActor> actor)
{
    assert(actor != NULL);
    _path->setActor(actor);
}   // end setActor


// public
void SurfacePathDrawer::setPathEndPoints( const QPoint& p0, const QPoint& p1)
{
    bool isshown = _path->getVisibility();
    _path->setVisibility(false);

    // Update the interpolated path over the surface of the face
    std::vector<cv::Point> handles(2);
    handles[0] = cv::Point( p0.x(), p0.y());
    handles[1] = cv::Point( p1.x(), p1.y());
    _path->setPathHandles(handles);

    // Sum over the actual length of the displayed curve
    std::vector<cv::Vec3f> pvs;
    const int n = _path->getAllPathVertices( pvs);
    assert( (int)pvs.size() == n);

    const double elen = cv::norm( *pvs.rbegin() - *pvs.begin());   // Euclidean length
    double psum = 0;
    for ( int i = 1; i < n; ++i)
        psum += cv::norm( pvs[i] - pvs[i-1]);

    // Set the caption's attachment point at start of the path.
    const cv::Vec3f& v0 = *pvs.begin();
    double attachPoint[3] = { v0[0], v0[1], v0[2]};
    _lenCaption->SetAttachmentPoint( attachPoint);

    // Set the text contents for the label and the caption
    std::ostringstream oss1, oss2;
    oss1 << "Caliper Distance = " << std::setw(5) << std::fixed << std::setprecision(1) << elen << " " << _munits << std::endl;
    oss2 << "Surface Distance = " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " " << _munits;
    _lenText->SetInput( (oss1.str() + oss2.str()).c_str());
    _lenText->SetDisplayPosition( _viewer->getWidth() - 10, 10);  // Bottom right

    // Stick a caption actor at the finishing point
    std::ostringstream caposs;
    caposs << std::fixed << std::setprecision(1) << elen << " " << _munits;
    _lenCaption->SetCaption( caposs.str().c_str());

    _path->setVisibility(isshown);
}   // end setPathEndPoints


// public slot
void SurfacePathDrawer::doDrawingPath( const QPoint& p)
{
    if ( _pathStart.x() < 0)
        _pathStart = p;

    if ( p != _pathStart)
    {
        setPathEndPoints( _pathStart, p);
        show(true);
    }   // end if
}   // end doDrawingPath


// public slot
void SurfacePathDrawer::doFinishedPath( const QPoint& p)
{
    _pathStart = QPoint(-1,-1);
    this->show(false);
}   // end doFinishedPath

