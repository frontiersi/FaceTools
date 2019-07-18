/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <VtkTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::LoopSelectVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::Vis::FV;
using FaceTools::FM;
using FaceTools::Action::Event;


LoopSelectVisualisation::LoopSelectVisualisation( double srad) : _srad(srad) {}


LoopSelectVisualisation::~LoopSelectVisualisation()
{
    while (!_lviews.empty())
        purge( const_cast<FV*>(_lviews.begin()->first), Event::NONE);
}   // end dtor


bool LoopSelectVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{   // Only check the centre reticule - outer ring is ignored.
    return (_sviews.count(fv) > 0) && (_sviews.at(fv)->prop() == prop);
}   // end belongs


void LoopSelectVisualisation::copy( FV* fv, const FV* s)
{
    purge(fv, Event::NONE);
    assert(_lviews.count(s) > 0);
    _lviews[fv] = new LoopsView( *_lviews.at(s));
    _sviews[fv] = new SphereView( *_sviews.at(s));
    setHighlighted( fv, false);
}   // end copy


void LoopSelectVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _lviews.count(fv) == 0)
    {
        _lviews[fv] = new LoopsView( 4.0f);
        _sviews[fv] = new SphereView( cv::Vec3f(0,0,0), _srad, true/*pickable*/, true/*fixed scale*/);
        _sviews[fv]->setResolution(101);
        _sviews[fv]->setOpacity(0.99);
    }   // end if
    _sviews.at(fv)->setVisible( true, fv->viewer());
    _lviews.at(fv)->setVisible( true, fv->viewer());
    setHighlighted( fv, false);
}   // end apply


bool LoopSelectVisualisation::purge( FV* fv, Event)
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
    return true;
}   // end purge


void LoopSelectVisualisation::setVisible( FV* fv, bool v)
{
    if ( _sviews.count(fv) > 0)
    {
        _sviews.at(fv)->setVisible( v, fv->viewer());
        _lviews.at(fv)->setVisible( v, fv->viewer());
    }   // end if
}   // end setVisible


bool LoopSelectVisualisation::isVisible( const FV* fv) const
{
    bool vis = false;
    if ( _sviews.count(fv) > 0)
    {
        vis = _sviews.at(fv)->visible();
        assert( vis ==_lviews.at(fv)->visible());
    }   // end if
    return vis;
}   // end isVisible


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


void LoopSelectVisualisation::setHighlighted( const FV* fv, bool hval)
{
    if ( _lviews.count(fv) > 0)
    {
        const cv::Vec3d col = hval ? cv::Vec3d(0.9, 0.2, 0.4) : cv::Vec3d(0.0, 0.6, 0.0);
        _sviews[fv]->setOpacity(0.5);
        _sviews[fv]->setColour( col[0], col[1], col[2]);
        _lviews[fv]->setColour( col[0], col[1], col[2]);
        _lviews[fv]->changeColour( col[0], col[1], col[2]);
    }   // end if
}   // end setHighlighted


void LoopSelectVisualisation::syncActorsToData(const FV *fv, const cv::Matx44d &d)
{
    if (_lviews.count(fv) > 0)
    {
        const cv::Matx44d& bmat = fv->data()->model().transformMatrix();
        const cv::Matx44d cm = d * bmat;
        vtkSmartPointer<vtkMatrix4x4> vm = RVTK::toVTK( cm);
        _sviews.at(fv)->setCentre( RFeatures::transform(cm, _sviews.at(fv)->centre()));
        _lviews.at(fv)->pokeTransform(vm);
    }   // end if
}   // end syncActorsToData
