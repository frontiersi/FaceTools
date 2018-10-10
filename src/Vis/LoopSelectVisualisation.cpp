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

#include <LoopSelectVisualisation.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <VtkTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LoopSelectVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using FaceTools::FaceModelViewer;
using FaceTools::Action::ActionVisualise;
using FaceTools::FM;


LoopSelectVisualisation::LoopSelectVisualisation( const QString& dname, const QIcon& icon, double srad)
    : BaseVisualisation(dname, icon), _srad(srad)
{
}   // end ctor


LoopSelectVisualisation::~LoopSelectVisualisation()
{
    while (!_lviews.empty())
        purge( const_cast<FV*>(_lviews.begin()->first));
}   // end dtor


bool LoopSelectVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{   // Only check the centre reticule - outer ring is ignored.
    return (_sviews.count(fv) > 0) && (_sviews.at(fv)->prop() == prop);
}   // end belongs


void LoopSelectVisualisation::copy( FV* fv, const FV* s)
{
    purge(fv);
    assert(_lviews.count(s) > 0);
    _lviews[fv] = new LoopsView( *_lviews.at(s));
    _sviews[fv] = new SphereView( *_sviews.at(s));
    _sviews[fv]->setResolution(101);
    setHighlighted( fv, false);
}   // end copy


void LoopSelectVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _lviews.count(fv) == 0)
    {
        _lviews[fv] = new LoopsView( 4.0f);
        _sviews[fv] = new SphereView( cv::Vec3f(0,0,0), _srad);
        _sviews[fv]->setResolution(101);
    }   // end if
    _sviews.at(fv)->setVisible( true, fv->viewer());
    _lviews.at(fv)->setVisible( true, fv->viewer());
    setHighlighted( fv, false);
}   // end apply


void LoopSelectVisualisation::clear( FV* fv)
{
    if ( _sviews.count(fv) > 0)
    {
        _sviews.at(fv)->setVisible( false, fv->viewer());
        _lviews.at(fv)->setVisible( false, fv->viewer());
    }   // end if
}   // end clear


void LoopSelectVisualisation::setReticule( const FV* fv, const cv::Vec3f& rpos)
{
    if ( _sviews.count(fv) > 0)
        _sviews.at(fv)->setCentre(rpos);
}   // end setReticule


void LoopSelectVisualisation::setPoints( const FV* fv, const std::vector<cv::Vec3f>& pts)
{
    if ( _lviews.count(fv) > 0)
    {
        const bool wasVisible = _lviews.at(fv)->visible();
        _lviews.at(fv)->setVisible( false, fv->viewer());
        _lviews.at(fv)->deleteActors();
        _lviews.at(fv)->addPoints( pts);
        _lviews.at(fv)->setVisible( wasVisible, fv->viewer());
    }   // end if
}   // end setPoints


void LoopSelectVisualisation::setPickable( const FV* fv, bool v)
{
    if ( _sviews.count(fv) > 0)
        _sviews.at(fv)->setPickable(v);
}   // end setPickable


bool LoopSelectVisualisation::setHighlighted( const FV* fv, bool hval)
{
    if ( _lviews.count(fv) == 0)
        return false;

    cv::Vec3f colour(0.0f, 0.6f, 0.0f); // non-highlight colour
    if ( hval)
        colour = cv::Vec3f( 0.9f, 0.2f, 0.4f);

    bool setcolour = false;
    if ( colour != _lviews[fv]->colour())
    {
        _sviews[fv]->setColour( colour[0], colour[1], colour[2]);
        _sviews[fv]->setOpacity(0.5);
        _lviews[fv]->setColour( colour);
        setcolour = true;
    }   // end if
    return setcolour;
}   // end setHighlighted


// protected
void LoopSelectVisualisation::pokeTransform( const FV* fv, const vtkMatrix4x4* vm)
{
    if (_lviews.count(fv) > 0)
    {
        _lviews.at(fv)->pokeTransform(vm);
        _sviews.at(fv)->pokeTransform(vm);
    }   // end if
}   // end pokeTransform


// protected
void LoopSelectVisualisation::fixTransform( const FV* fv)
{
    if (_lviews.count(fv) > 0)
    {
        _lviews.at(fv)->fixTransform();
        _sviews.at(fv)->fixTransform();
    }   // end if
}   // end fixTransform


// protected
void LoopSelectVisualisation::purge( FV* fv)
{
    if (_lviews.count(fv) > 0)
    {
        _lviews.at(fv)->setVisible( false, fv->viewer());
        delete _lviews.at(fv);
        _lviews.erase(fv);
        _sviews.at(fv)->setVisible( false, fv->viewer());
        delete _sviews.at(fv);
        _sviews.erase(fv);
    }   // end if
}   // end purge

