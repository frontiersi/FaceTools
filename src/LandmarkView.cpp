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
using FaceTools::ModelViewerAnnotator;
using FaceTools::VisualisationOptions;
using namespace FaceTools::Landmarks;


LandmarkView::LandmarkView( ModelViewer* viewer, const FaceTools::Landmarks::Landmark* lm, const VisualisationOptions::Landmarks& visopts)
    : _viewer(viewer),
      _annotator( new ModelViewerAnnotator( viewer->getRenderer())),
      _landmark(lm),
      _source( vtkSmartPointer<vtkSphereSource>::New()),
      _actor( vtkSmartPointer<vtkActor>::New()),
      _ishighlighted(false), _msgID(0), _isshown(false)
{
    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _source->GetOutputPort());
    _actor->SetMapper(mapper);
    update();
    setVisualisationOptions(visopts);
}   // end ctor


LandmarkView::~LandmarkView()
{
    show(false);
    _annotator->removeMessage( _msgID);
    delete _annotator;
}   // end dtor


void LandmarkView::show( bool enable)
{
    if ( enable && _landmark->visible)
    {
        if ( !_isshown)
            _viewer->add(_actor);
        _isshown = true;
    }   // end if
    else
    {
        if ( _isshown)
            _viewer->remove(_actor);
        _isshown = false;
    }   // end else
}   // end show


bool LandmarkView::isShown() const
{
    return _isshown;
}   // end isShown


void LandmarkView::update()
{
    _source->SetCenter( _landmark->pos[0], _landmark->pos[1], _landmark->pos[2]);
    _source->Update();
    _annotator->removeMessage( _msgID);
    if ( _ishighlighted && _landmark->visible)
        _msgID = _annotator->showMessage( 0.95, 0.05, ModelViewerAnnotator::RightJustify, _landmark->name);
    _viewer->updateRender();
}   // end update


void LandmarkView::highlight( bool enable)
{
    _ishighlighted = enable;
    double rad;
    QColor col;

    if ( _ishighlighted)
    {
        col = _visopts.highlightColour;
        rad = _visopts.highlightRadius;
        _msgID = _annotator->showMessage( 0.95, 0.05, ModelViewerAnnotator::RightJustify, _landmark->name);
    }   // end if
    else
    {
        col = _visopts.colour;
        rad = _visopts.radius;
        _annotator->removeMessage( _msgID);
    }   // end else

    _actor->GetProperty()->SetColor( col.redF(), col.greenF(), col.blueF());
    _actor->GetProperty()->SetOpacity( col.alphaF());
    _source->SetRadius( rad);
}   // end highlight


void LandmarkView::setVisualisationOptions( const VisualisationOptions::Landmarks& visopts)
{
    _visopts = visopts;
    highlight( _ishighlighted);
}   // end setVisualisationOptions


bool LandmarkView::isPointedAt( const QPoint& p) const
{
    return _viewer->getPointedAt(p, _actor);
}   // end isPointedAt
