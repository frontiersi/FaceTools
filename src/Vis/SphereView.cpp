/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <Transformer.h>    // RFeatures
#include <VtkTools.h>       // RVTK
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
using QTools::VtkScalingActor;
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;

// private
void SphereView::init()
{
    _visible = false;
    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOff();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(21);
    _caption->GetCaptionTextProperty()->SetColor( 1,1,1);
    //_caption->GetCaptionTextProperty()->SetUseTightBoundingBox(true);
    _caption->SetVisibility(false);
    _caption->SetPickable(false);

    _actor = new VtkScalingActor( _source);
}   // end init


SphereView::SphereView( const cv::Vec3f& c, double r, bool p, bool fixed)
{
    init();
    setCentre(c);
    setRadius(r);
    setPickable(p);
    setFixedScale(fixed);
}   // end ctor


SphereView::SphereView( const SphereView& sv)
{
    *this = sv;
}   // end ctor


SphereView& SphereView::operator=( const SphereView& sv)
{
    init();
    _visible = sv._visible;
    setResolution( sv.resolution());
    setPickable( sv.pickable());
    setFixedScale( sv.fixedScale());
    setScaleFactor( sv.scaleFactor());
    setCentre( sv.centre());
    setRadius( sv.radius());
    setOpacity( sv.opacity());
    setColour( sv.colour());
    setCaption( sv.caption());
    setHighlighted( sv.highlighted());
    return *this;
}   // end operator=


SphereView::~SphereView() { delete _actor;}   // end dtor

void SphereView::setResolution( int t)
{
    t = std::max<int>(t,8);
    _source->SetPhiResolution(t);
    _source->SetThetaResolution(int(double(t+1)/2));
}   // end setResolution

int SphereView::resolution() const { return _source->GetPhiResolution();}

void SphereView::setPickable( bool v) { _actor->setPickable(v);}
bool SphereView::pickable() const { return _actor->pickable();}

void SphereView::setFixedScale( bool v) { _actor->setFixedScale(v);}
bool SphereView::fixedScale() const { return _actor->fixedScale();}

void SphereView::setScaleFactor( double v) { _actor->setScaleFactor(v);}
double SphereView::scaleFactor() const { return _actor->scaleFactor();}

void SphereView::setCentre( const cv::Vec3f& pos)
{
    _actor->setPosition(pos);
    setHighlighted(highlighted());  // Just for resetting the caption attachment point
}   // end setCentre

cv::Vec3f SphereView::centre() const { return _actor->position();}

void SphereView::setRadius( double r) { _source->SetRadius(r);}
double SphereView::radius() const { return _source->GetRadius();}

void SphereView::setOpacity( double v) { _actor->setOpacity(v);}
double SphereView::opacity() const { return _actor->opacity();}

void SphereView::setColour( double r, double g, double b) { _actor->setColour(r,g,b);}
void SphereView::setColour( const double c[]) { _actor->setColour(c);}
const double* SphereView::colour() const { return _actor->colour();}

void SphereView::setCaption( const std::string& lname)
{
    _caption->SetCaption( lname.c_str());
    _caption->SetPosition2( lname.size() * 0.015, 0.05);
    //_caption->SetHeight(0.05);
}   // end setCaption


std::string SphereView::caption() const
{
    const char* cap = _caption->GetCaption();
    return cap == nullptr ? "" : cap;
}   // end caption

void SphereView::setHighlighted( bool v)
{
    const cv::Vec3f& pos = _actor->position();
    double attachPoint[3] = {double(pos[0]), double(pos[1]), double(pos[2])};
    _caption->SetAttachmentPoint( attachPoint);
    _caption->SetVisibility( v);
}   // end setHighlighted

bool SphereView::highlighted() const { return static_cast<bool>(_caption->GetVisibility());}

const vtkProp* SphereView::prop() const { return _actor->prop();}

void SphereView::setVisible( bool v, ModelViewer* vwr)
{
    assert(vwr);
    vwr->remove( _actor->prop());
    vwr->remove( _caption);
    _actor->setRenderer( nullptr);
    if ( v)
    {
        _actor->setRenderer( vwr->getRenderer());
        vwr->add( _actor->prop());
        vwr->add( _caption);
    }   // end if
    _visible = v;
}   // end setVisible

void SphereView::pokeTransform( const vtkMatrix4x4* vm) { _actor->pokeTransform(vm);}

void SphereView::fixTransform()
{
    _actor->fixTransform();
}   // end fixTransform
