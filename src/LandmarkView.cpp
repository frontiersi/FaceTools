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
#include <vtkPolyDataMapper.h>
#include <vtkMapper.h>
using FaceTools::LandmarkView;
using FaceTools::ModelViewer;
using FaceTools::ModelOptions;
using namespace FaceTools::Landmarks;


LandmarkView::LandmarkView( const FaceTools::Landmarks::Landmark* lm, const ModelOptions::Landmarks& opts)
    : _viewer(NULL),
      _landmark(lm),
      _source( vtkSmartPointer<vtkSphereSource>::New()),
      _actor( vtkSmartPointer<vtkActor>::New()),
      _ishighlighted(false), _isshown(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);
    update();
    setOptions(opts);
}   // end ctor


LandmarkView::~LandmarkView()
{
    setVisible( false, NULL);
}   // end dtor


void LandmarkView::setVisible( bool enable, ModelViewer* viewer)
{
    if ( _viewer)
        _viewer->remove(_actor);

    if ( viewer)
        viewer->remove(_actor);

    _isshown = false;
    _viewer = viewer;
    if ( enable && viewer)
    {
        viewer->add(_actor);
        _isshown = true;
    }   // end if
}   // end setVisible


bool LandmarkView::isVisible() const { return _isshown;}


void LandmarkView::update()
{
    _source->SetCenter( _landmark->pos[0], _landmark->pos[1], _landmark->pos[2]);
    _source->Update();
}   // end update


void LandmarkView::highlight( bool enable)
{
    QColor col = _opts.colour;
    double rad = _opts.radius;
    _ishighlighted = enable && isVisible();
    if ( _ishighlighted)
    {
        col = _opts.highlightColour;
        rad = _opts.highlightRadius;
    }   // end if
    _actor->GetProperty()->SetColor( col.redF(), col.greenF(), col.blueF());
    _actor->GetProperty()->SetOpacity( col.alphaF());
    _source->SetRadius( rad);
}   // end highlight


void LandmarkView::setOptions( const ModelOptions::Landmarks& opts)
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
