/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Vis/SphereView.h>
#include <r3dvis/VtkTools.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkTextActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
#include <algorithm>
#include <cassert>
using r3dvis::VtkScalingActor;
using FaceTools::Vis::SphereView;
using FaceTools::ModelViewer;
using FaceTools::Vec3f;

void SphereView::_init( const Vec3f& c, float r, bool p, bool fixed)
{
    if ( _actor)
        delete _actor;
    _actor = new VtkScalingActor( _source.Get());

    setCentre(c);
    setRadius(r);
    setPickable(p);
    setFixedScale(fixed);
    setVisible( _visible, _vwr);

    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOn();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(21);
    setCaptionColour( Qt::GlobalColor::white);
    _caption->GetCaptionTextProperty()->SetUseTightBoundingBox(true);
    _caption->SetVisibility(false);
    _caption->SetPickable(false);
    _caption->GetTextActor()->SetTextScaleModeToNone();
}   // end _init


SphereView::SphereView( const Vec3f& c, float r, bool p, bool fixed)
    : _vwr(nullptr), _visible(false), _actor(nullptr)
{
    _init( c, r, p, fixed);
}   // end ctor


SphereView::~SphereView()
{
    delete _actor;
}   // end dtor


void SphereView::setResolution( int t)
{
    t = std::max<int>(t,8);
    _source->SetPhiResolution(t);
    _source->SetThetaResolution(int(float(t+1)/2));
}   // end setResolution

int SphereView::resolution() const { return _source->GetPhiResolution();}

void SphereView::setPickable( bool v) { _actor->setPickable(v);}
bool SphereView::pickable() const { return _actor->pickable();}

void SphereView::setFixedScale( bool v) { _actor->setFixedScale(v);}
bool SphereView::fixedScale() const { return _actor->fixedScale();}

void SphereView::setScaleFactor( float v) { _actor->setScaleFactor(v);}
float SphereView::scaleFactor() const { return _actor->scaleFactor();}

void SphereView::setCentre( const Vec3f& pos)
{
    //r3d::Vec3d dpos = pos.cast<double>();
    //_source->SetCenter( &dpos[0]);
    _actor->setPosition( pos);
    _updateCaptionPosition();
}   // end setCentre

const Vec3f& SphereView::centre() const { return _actor->position();}

void SphereView::setRadius( float r) { _source->SetRadius(r);}
float SphereView::radius() const { return _source->GetRadius();}

float SphereView::opacity() const { return _actor->opacity();}

void SphereView::setColour( double r, double g, double b, double a)
{
    _actor->setColour(r,g,b);
    _actor->setOpacity(a);
}   // end setColour

const double* SphereView::colour() const { return _actor->colour();}

void SphereView::setCaption( const std::string& lname) { _caption->SetCaption( lname.c_str());}
void SphereView::setCaption( const QString& lname) { setCaption(lname.toStdString());}

std::string SphereView::caption() const
{
    const char* cap = _caption->GetCaption();
    return cap == nullptr ? "" : cap;
}   // end caption

void SphereView::setCaptionColour( const QColor& tcol)
{
    assert(_caption != nullptr);
    _caption->GetCaptionTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
}   // end setCaptionColour

void SphereView::showCaption( bool v)
{
    _caption->SetVisibility( v);
    _updateCaptionPosition();
}   // end showCaption

//bool SphereView::highlighted() const { return static_cast<bool>(_caption->GetVisibility());}

const vtkProp* SphereView::prop() const { return _actor->prop();}

void SphereView::setVisible( bool v, ModelViewer* vwr)
{
    if ( !v)
    {
        if ( _vwr)
        {
            _vwr->remove( _actor->prop());
            _vwr->remove( _caption);
        }   // end if
        if ( vwr)
        {
            vwr->remove( _actor->prop());
            vwr->remove( _caption);
        }   // end if

        _actor->setRenderer( nullptr);
    }   // end if

    _vwr = vwr;

    if ( v && _vwr)
    {
        _actor->setRenderer( _vwr->getRenderer());
        _vwr->add( _actor->prop());
        _vwr->add( _caption);
        _visible = true;
    }   // end if
}   // end setVisible


bool SphereView::belongs( const vtkProp *prop) const { return prop == _actor->prop();}


void SphereView::pokeTransform( const vtkMatrix4x4* d)
{
    vtkMatrix4x4* vm = const_cast<vtkMatrix4x4*>(d);
    _actor->pokeTransform( vm);
    _updateCaptionPosition();
}   // end pokeTransform


const vtkMatrix4x4* SphereView::transform() const { return _actor->transform();}


void SphereView::_updateCaptionPosition()
{
    const Vec3f pos = r3d::transform( r3dvis::toEigen(_actor->transform()), centre());
    double attachPoint[3] = {double(pos[0]), double(pos[1]), double(pos[2])};
    _caption->SetAttachmentPoint( attachPoint);
}   // end _updateCaptionPosition
