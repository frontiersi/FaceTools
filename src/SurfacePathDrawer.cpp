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


SurfacePathDrawer::SurfacePathDrawer( FaceTools::ModelViewer* mv, const std::string& munits)
    : _munits(munits),
      _viewer(mv),
      _lenText( vtkSmartPointer<vtkTextActor>::New()),
      _lenCaption( vtkSmartPointer<vtkCaptionActor2D>::New()),
      _pathStart(NULL),
      _actor(NULL)
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
}   // end ctor


SurfacePathDrawer::~SurfacePathDrawer()
{
}   // end dtor


// public slot
void SurfacePathDrawer::setModel( const vtkActor* actor)
{
    _actor= actor;
    _path.reset();
    if ( _pathStart)
        delete _pathStart;
    _pathStart = NULL;

    if ( _actor)
    {
        _path = RVTK::ModelPathDrawer::create( _viewer->getRenderWindow()->GetInteractor());
        _path->setClosed(false);    // A line segment rather than a boundary
        _path->setModel(_actor);
    }   // end if
}   // end setModel


// public
bool SurfacePathDrawer::isShown() const
{
    return _path && _path->getVisibility();
}   // end isShown


// public
void SurfacePathDrawer::show( bool enable)
{
    if ( !_path)
        return;
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
void SurfacePathDrawer::setPathEndPoints( const cv::Vec3f& v0, const cv::Vec3f& v1)
{
    assert(_path);
    // Update the interpolated path over the surface of the face
    std::vector<cv::Vec3f> handles(2);
    handles[0] = v0; handles[1] = v1;
    _path->setPathHandles(handles);

    const double eucLen = cv::norm( v1 - v0);   // Euclidean length

    // Sum over the actual length of the displayed curve
    std::vector<cv::Vec3f> pvs;
    _path->getAllPathVertices( pvs);
    double psum = 0;
    cv::Vec3f* prevVert = &pvs[0];
    const int nverts = (int)pvs.size();
    for ( int i = 1; i < nverts; ++i)
    {
        psum += cv::norm( pvs[i] - *prevVert);
        prevVert = &pvs[i];
    }   // end for

    // Set the caption's attachment point
    double attachPoint[3] = { v1[0], v1[1], v1[2]};
    _lenCaption->SetAttachmentPoint( attachPoint);

    // Set the text contents for the label and the caption
    std::ostringstream oss1, oss2;
    oss1 << "Caliper Distance = " << std::setw(5) << std::fixed << std::setprecision(1) << eucLen << " " << _munits << std::endl;
    oss2 << "Surface Distance = " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " " << _munits;
    _lenText->SetInput( (oss1.str() + oss2.str()).c_str());
    _lenText->SetDisplayPosition( _viewer->getWidth() - 10, 10);  // Bottom right

    // Stick a caption actor at the finishing point
    std::ostringstream caposs;
    caposs << std::fixed << std::setprecision(1) << eucLen << " " << _munits;
    _lenCaption->SetCaption( caposs.str().c_str());
}   // end setPathEndPoints


// public slot
void SurfacePathDrawer::doDrawingPath( const QPoint& p)
{
    if ( !_actor)
        return;

    cv::Vec3f curPos;
    cv::Vec3f* wpos = &curPos;
    if ( _pathStart == NULL)
    {
        _pathStart = new cv::Vec3f;
        wpos = _pathStart;
    }   // end if

    if ( _viewer->calcSurfacePosition( _actor, p, *wpos))
    {
        setPathEndPoints( *_pathStart, *wpos);
        show(true);
    }   // end if
}   // end doDrawingPath


// public slot
void SurfacePathDrawer::doFinishedPath( const QPoint& p)
{
    if ( _pathStart)
        delete _pathStart;
    _pathStart = NULL;
    this->show(false);
}   // end doFinishedPath

