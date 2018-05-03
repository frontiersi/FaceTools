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

#include <SurfacePath.h>
#include <FaceModelViewer.h>
#include <FaceView.h>
#include <FaceModel.h>
#include <VtkTools.h>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
using FaceTools::SurfacePath;
using FaceTools::FaceControl;
using FaceTools::ModelViewer;
using RFeatures::ObjModel;


// public
SurfacePath::SurfacePath( const FaceControl* fcont, const QPoint& p)
    : _fcont(fcont), _h0(NULL), _h1(NULL), _visible(false), _pathID(-1),
      _lenText( vtkSmartPointer<vtkTextActor>::New()),
      _lenCaption( vtkSmartPointer<vtkCaptionActor2D>::New())
{
    _lenText->GetTextProperty()->SetJustificationToRight();
    _lenText->GetTextProperty()->SetFontFamilyToCourier();
    _lenText->GetTextProperty()->SetFontSize(15);

    _lenCaption->BorderOff();
    _lenCaption->GetCaptionTextProperty()->BoldOff();
    _lenCaption->GetCaptionTextProperty()->ItalicOff();
    _lenCaption->GetCaptionTextProperty()->ShadowOff();
    _lenCaption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _lenCaption->GetCaptionTextProperty()->SetFontSize(6);
    _lenCaption->SetPickable( false);
    _lenCaption->SetLayerNumber(1);

    _h1 = new Handle(this);
    _h0 = new Handle(this);
    _h1->set(p);
    _h0->set(p);
}   // end ctor


// public
SurfacePath::~SurfacePath()
{
    removePath();
    delete _h0;
    delete _h1;
}   // end dtor


bool SurfacePath::isVisible() const { return _visible;}


// public
void SurfacePath::setVisible( bool enable)
{
    removePath();
    _visible = false;
    if ( enable)
    {
        addPath();
        _visible = true;
    }   // end if
}   // end setVisible


// public
SurfacePath::Handle* SurfacePath::handle( const QPoint& p) const
{
    Handle* handle = NULL;
    const vtkProp* prop = _fcont->viewer()->getPointedAt(p);
    if ( _h0 && prop == _h0->_actor)
        handle = _h0;
    else if ( _h1 && prop == _h1->_actor)
        handle = _h1;
    return handle;
}   // end handle


// private
void SurfacePath::removePath()
{
    ModelViewer* viewer = _fcont->viewer();
    viewer->remove(_h0->_actor);
    viewer->remove(_h1->_actor);
    if ( _pathID >= 0)
        viewer->remove(_pathID);
    viewer->remove(_lenText);
    viewer->remove(_lenCaption);
    _pathID = -1;
}   // end removePath


// private
void SurfacePath::addPath()
{
    if ( createPath())
    {
        const ModelViewer::VisOptions loptions( 1.0f,1.0f,1.0f,1.0f,false,1.0f,4.0f);
        ModelViewer* viewer = _fcont->viewer();
        _pathID = viewer->addLine( _spfinder.lastPath(), false, loptions);
        viewer->add(_h0->_actor);
        viewer->add(_h1->_actor);
        viewer->add(_lenText);
        viewer->add(_lenCaption);
    }   // end if
}   // end addPath


SurfacePath::Handle::Handle( SurfacePath* spd)
    : _host(spd)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    _source = vtkSmartPointer<vtkSphereSource>::New();
    _actor = vtkSmartPointer<vtkActor>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);
    highlight(false);
}   // end ctor


// public
void SurfacePath::Handle::set( const QPoint& p)
{
    cv::Vec3f wpos;
    const vtkActor* prop = _host->_fcont->view()->getSurfaceActor();
    _host->_fcont->viewer()->calcSurfacePosition( prop, p, wpos);
    _source->SetCenter( wpos[0], wpos[1], wpos[2]);
    _source->Update();
    _host->removePath();
    _host->addPath();
}   // end set


// public
cv::Vec3f SurfacePath::Handle::pos() const
{
    const double *c = _source->GetCenter();
    return cv::Vec3f( (float)c[0], (float)c[1], (float)c[2]);
}   // end pos


// public
void SurfacePath::Handle::highlight( bool v)
{
    double opacity = 0.6;
    double rad = 2.6;
    double cols[3] = {0.5,0.2,1.0};
    if (v)
    {
        cols[0] = 0.5;
        cols[1] = 0.1;
        cols[2] = 0.7;
        opacity = 0.2;
    }   // end if
    _highlighted = v;
    _actor->GetProperty()->SetColor( cols[0], cols[1], cols[2]);
    _actor->GetProperty()->SetOpacity( opacity);
    _source->SetRadius( rad);
}   // end highlight


// private
bool SurfacePath::createPath()
{
    // Find shortest path over the surface (NOT GEODESIC!)
    const double psum = _spfinder.findPath( _fcont->data()->model(), _h0->pos(), _h1->pos());
    if ( psum <= 0.0)
        return false;

    const std::vector<cv::Vec3f>& lpath = _spfinder.lastPath();
    // Calculate the Euclidean length between end points
    const double elen = cv::norm( lpath.front() - lpath.back());

    // Set the caption's attachment point in the middle of the path
    const cv::Vec3f& av = lpath[lpath.size()/2];
    double attachPoint[3] = { av[0], av[1], av[2]};
    _lenCaption->SetAttachmentPoint( attachPoint);
    const FaceTools::ModelOptions& opts = _fcont->options();
    const QColor& tcol = opts.textColour;
    _lenCaption->GetCaptionTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
    _lenCaption->SetVisibility( opts.showCaptions);

    // Set the text contents for the label and the caption
    std::ostringstream oss1, oss2;
    oss1 << "Caliper: " << std::setw(5) << std::fixed << std::setprecision(1) << elen << " " << opts.munits<< std::endl;
    oss2 << "Geodesic: " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " " << opts.munits;
    _lenText->SetInput( (oss1.str() + oss2.str()).c_str());
    _lenText->SetDisplayPosition( (int)_fcont->viewer()->getWidth() - 10, 10);  // Bottom right
    _lenText->GetTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());

    std::ostringstream caposs;
    caposs << std::fixed << std::setprecision(1) << elen << " " << opts.munits;
    _lenCaption->SetCaption( caposs.str().c_str());
    return true;
}   // end createPath

