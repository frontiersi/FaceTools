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

#include <LandmarkView.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
using FaceTools::LandmarkView;
using FaceTools::ModelViewer;
using FaceTools::ModelOptions;


LandmarkView::LandmarkView( const ModelOptions& opts)
    : _viewer(NULL),
      _source( vtkSmartPointer<vtkSphereSource>::New()),
      _actor( vtkSmartPointer<vtkActor>::New()),
      _caption( vtkSmartPointer<vtkCaptionActor2D>::New()),
      _ishighlighted(false), _isshown(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);

    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOff();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(4);
    _caption->GetCaptionTextProperty()->SetUseTightBoundingBox(true);
    _caption->SetPickable(false);

    set( "NO_NAME", cv::Vec3f(0.0f, 0.0f, 0.0f));
    setOptions(opts);
}   // end ctor


LandmarkView::~LandmarkView()
{
    setVisible( false, NULL);
}   // end dtor


void LandmarkView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
    {
        _viewer->remove(_actor);
        _viewer->remove(_caption);
    }   // end if

    if ( viewer)
    {
        viewer->remove(_actor);
        viewer->remove(_caption);
    }   // end if

    _isshown = false;
    _viewer = viewer;
    if ( enable && viewer)
    {
        viewer->add(_actor);
        _isshown = true;
    }   // end if
}   // end setVisible


bool LandmarkView::isVisible() const { return _isshown;}


void LandmarkView::set( const std::string& lname, const cv::Vec3f& pos)
{
    _source->SetCenter( pos[0], pos[1], pos[2]);
    _source->Update();
    _caption->SetCaption( lname.c_str());
    double attachPoint[3] = {pos[0], pos[1], pos[2]};
    _caption->SetAttachmentPoint( attachPoint);
}   // end set


void LandmarkView::highlight( bool enable)
{
    QColor col = _opts.landmarks.colour;
    double opacity = 1.0;
    double rad = _opts.landmarks.radius;
    _ishighlighted = enable && isVisible();
    if ( _ishighlighted)
        opacity = 0.4;
    _actor->GetProperty()->SetColor( col.redF(), col.greenF(), col.blueF());
    _actor->GetProperty()->SetOpacity( opacity);
    _source->SetRadius( rad);

    const QColor& tcol = _opts.textColour;
    _caption->GetCaptionTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
    _caption->SetVisibility( _opts.showCaptions);

    if ( _viewer)
    {
        _viewer->remove(_caption);
        if ( _ishighlighted)
            _viewer->add(_caption);
    }   // end if
}   // end highlight


void LandmarkView::setOptions( const ModelOptions& opts)
{
    _opts = opts;
    highlight( _ishighlighted);
}   // end setOptions


bool LandmarkView::isPointedAt( const QPoint& p) const
{
    bool pointedAt = false;
    if ( _viewer)
        pointedAt = _viewer->getPointedAt(p, _actor);
    return pointedAt;
}   // end isPointedAt


bool LandmarkView::isProp( const vtkProp* prop) const { return _actor == prop;}

