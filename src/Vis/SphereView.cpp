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

#include <SphereView.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;


SphereView::SphereView( const cv::Vec3f& c, double r, bool p)
    : _viewer(nullptr),
      _source( vtkSmartPointer<vtkSphereSource>::New()),
      _actor( vtkSmartPointer<vtkActor>::New()),
      _caption( vtkSmartPointer<vtkCaptionActor2D>::New()),
      _ishighlighted(false), _isshown(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);

    vtkProperty* property = _actor->GetProperty();
    property->SetAmbient( 1.0);
    property->SetDiffuse( 0.0);
    property->SetSpecular( 0.0);

    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOff();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(6);
    _caption->GetCaptionTextProperty()->SetColor( 1,1,1);
    _caption->GetCaptionTextProperty()->SetUseTightBoundingBox(true);
    _caption->SetPickable(false);

    setCentre(c);
    setRadius(r);
    setPickable(p);
}   // end ctor


SphereView::~SphereView()
{
    setVisible( false, nullptr);
}   // end dtor


void SphereView::setResolution( int t)
{
    t = std::max<int>(t,8);
    _source->SetPhiResolution(t);
    _source->SetThetaResolution((int)(double(t+1)/2));
}   // end setResolution


int SphereView::resolution() const { return _source->GetPhiResolution();}


// public
void SphereView::setPickable( bool v) { _actor->SetPickable(v);}
bool SphereView::pickable() const { return _actor->GetPickable();}
double SphereView::opacity() const { return _actor->GetProperty()->GetOpacity();}
void SphereView::setOpacity( double v) { _actor->GetProperty()->SetOpacity( v);}
double SphereView::radius() const { return _source->GetRadius();}

void SphereView::setColour( double r, double g, double b) { _actor->GetProperty()->SetColor(r,g,b);}
const double* SphereView::colour() const { return _actor->GetProperty()->GetColor();}

void SphereView::setCaption( const std::string& lname) { _caption->SetCaption( lname.c_str());}

// public
void SphereView::setCentre( const cv::Vec3f& pos)
{
    _source->SetCenter( pos[0], pos[1], pos[2]);
    _source->Update();
    double attachPoint[3] = {pos[0], pos[1], pos[2]};
    _caption->SetAttachmentPoint( attachPoint);
}   // end setCentre


// public
cv::Vec3f SphereView::centre() const
{
    double vp[3];
    _source->GetCenter( vp);
    return cv::Vec3f( float(vp[0]), float(vp[1]), float(vp[2]));
}   // end centre


// public
void SphereView::pokeTransform( const vtkMatrix4x4* vm)
{
    _actor->PokeMatrix( const_cast<vtkMatrix4x4*>(vm));
    double vp[3];   // Get the translation vector from the transform matrix
    vp[0] = vm->GetElement(0,3);
    vp[1] = vm->GetElement(1,3);
    vp[2] = vm->GetElement(2,3);
    _caption->SetAttachmentPoint( vp);
}   // end pokeTransform


// public
void SphereView::fixTransform()
{
    RVTK::transform( _actor, _actor->GetMatrix());
}   // end fixTransform


// public
void SphereView::setRadius( double r)
{
    _source->SetRadius(r);
    _source->Update();
}   // end setRadius


// public
void SphereView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
    {
        _viewer->remove(_actor);
        _viewer->remove(_caption);
    }   // end if

    _isshown = false;
    _ishighlighted = false;
    _viewer = viewer;
    if ( enable && _viewer)
    {
        _viewer->add(_actor);
        _isshown = true;
    }   // end if
}   // end setVisible


// public
bool SphereView::isVisible() const { return _isshown;}
bool SphereView::isHighlighted() const { return _ishighlighted;}
bool SphereView::isProp( const vtkProp* prop) const { return _actor == prop;}


// public
void SphereView::highlight( bool enable)
{
    _ishighlighted = enable && isVisible();

    if ( _viewer)
    {
        _viewer->remove(_caption);
        if ( _ishighlighted)
            _viewer->add(_caption);
    }   // end if
}   // end highlight


bool SphereView::pointedAt( const QPoint& p) const
{
    bool pointedAt = false;
    if ( _viewer)
        pointedAt = _viewer->getPointedAt( &p, _actor);
    return pointedAt;
}   // end pointedAt
