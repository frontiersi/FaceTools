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

#include <PathSetVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Path;


PathSetVisualisation::PathSetVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon),
      _caption( vtkSmartPointer<vtkCaptionActor2D>::New()),
      _text( vtkSmartPointer<vtkTextActor>::New())
{
    // The caption for the current path.
    _caption->BorderOff();
    _caption->GetCaptionTextProperty()->BoldOff();
    _caption->GetCaptionTextProperty()->ItalicOff();
    _caption->GetCaptionTextProperty()->ShadowOff();
    _caption->GetCaptionTextProperty()->SetFontFamilyToCourier();
    _caption->GetCaptionTextProperty()->SetFontSize(8);
    _caption->SetPickable( false);
    _caption->SetLayerNumber(1);

    // The bottom right text.
    _text->GetTextProperty()->SetJustificationToRight();
    _text->GetTextProperty()->SetFontFamilyToCourier();
    _text->GetTextProperty()->SetFontSize(16);
    _text->SetPickable( false);

    QColor tcol(255,255,255);
    _caption->GetCaptionTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
    _text->GetTextProperty()->SetColor( tcol.redF(), tcol.greenF(), tcol.blueF());
}   // end ctor


PathSetVisualisation::~PathSetVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool PathSetVisualisation::belongs( const vtkProp* prop, const FaceControl* fc) const
{
    return pathHandle( fc, prop) != nullptr;
}   // end belongs


bool PathSetVisualisation::apply( const FaceControl* fc, const QPoint*)
{
    if ( _views.count(fc) == 0)
    {
        const FaceModel* fm = fc->data();
        FaceTools::PathSet::Ptr paths = fm->paths();
        _views[fc] = new PathSetView( paths);
    }   // end if
    return true;
}   // end apply


void PathSetVisualisation::addActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        FaceModelViewer* viewer = fc->viewer();
        _views.at(fc)->setVisible( true, viewer);
        viewer->add( _caption);
        viewer->add( _text);
    }   // end if
}   // end addActors


void PathSetVisualisation::removeActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        FaceModelViewer* viewer = fc->viewer();
        _views.at(fc)->setVisible( false, viewer);
        viewer->remove( _caption);
        viewer->remove( _text);
    }   // end if
}   // end removeActors


const PathView::Handle* PathSetVisualisation::addPath( const FaceControl* fc, int pathId)
{
    assert( _views.count(fc) > 0);
    PathView::Handle *h = nullptr;
    if (_views.count(fc) > 0)
    {
        h = _views.at(fc)->addPath( pathId);
        _views.at(fc)->showPath( true, pathId);
    }   // end if
    return h;
}   // end addPath


const PathView::Handle* PathSetVisualisation::pathHandle( const FaceControl* fc, const vtkProp* prop) const
{
    PathView::Handle *h = nullptr;
    if (_views.count(fc) > 0)
        h = _views.at(fc)->handle( prop);
    return h;
}   // end pathHandle


const PathView::Handle* PathSetVisualisation::pathHandle0( const FaceControl* fc, int pid) const
{
    PathView::Handle* h = nullptr;
    if (_views.count(fc) > 0)
    {
        PathView* pv = _views.at(fc)->pathView(pid);
        if ( pv)
            h = pv->handle0();
    }   // end if
    return h;
}   // end pathHandle0


const PathView::Handle* PathSetVisualisation::pathHandle1( const FaceControl* fc, int pid) const
{
    PathView::Handle* h = nullptr;
    if (_views.count(fc) > 0)
    {
        PathView* pv = _views.at(fc)->pathView(pid);
        if ( pv)
            h = pv->handle1();
    }   // end if
    return h;
}   // end pathHandle1


// public
void PathSetVisualisation::updatePath( const FaceControl* fc, int pathId)
{
    assert( _views.count(fc) > 0);
    if (_views.count(fc) > 0)
        _views.at(fc)->updatePath( pathId);
}   // end updatePath


// public
void PathSetVisualisation::setCaptions( const FaceControl* fc, int pathId, const cv::Vec3f* attachPoint)
{
    assert( _views.count(fc) > 0);
    if (_views.count(fc) > 0)
    {
        const PathSetView* psv = _views.at(fc);

        // Read the path lengths and attachment points.
        const Path* path = fc->data()->paths()->path(pathId);
        const double psum = path->psum;
        const double elen = path->elen;
        assert( !path->vtxs.empty());
        cv::Vec3f av = path->vtxs[path->vtxs.size()/2]; // Attach halfway along ...
        if ( attachPoint)   // ... unless attachment point given
            av = *attachPoint;

        // Set the caption's attachment point in the middle of the path
        double attachPoint[3] = { av[0], av[1], av[2]};
        _caption->SetAttachmentPoint( attachPoint);

        // Set the text contents for the label and the caption
        std::ostringstream oss0, oss1, oss2;
        if ( !path->name.empty())
            oss0 << path->name << std::endl;
        oss1 << "Caliper: " << std::setw(5) << std::fixed << std::setprecision(1) << elen << " mm" << std::endl;
        oss2 << "Surface: " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " mm";
        _text->SetInput( (oss0.str() + oss1.str() + oss2.str()).c_str());
        _text->SetDisplayPosition( (int)fc->viewer()->getWidth() - 10, 10);  // Bottom right

        std::ostringstream caposs;
        caposs << std::fixed << std::setprecision(1) << elen << " mm";
        _caption->SetCaption( caposs.str().c_str());
    }   // end if
}   // end setCaptions


// public
void PathSetVisualisation::setCaptionsVisible( bool v)
{
    _caption->SetVisibility( v);
    _text->SetVisibility( v);
}   // end setCaptionsVisible


// protected
void PathSetVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* m)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->pokeTransform(m);
    //double attachPoint[3] = { m->GetElement(0,3), m->GetElement(1,3), m->GetElement(2,3)};
    //_caption->SetAttachmentPoint( attachPoint);
}   // end pokeTransform


// protected
void PathSetVisualisation::fixTransform( const FaceControl* fc)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->fixTransform();
}   // end fixTransform


// protected
void PathSetVisualisation::purge( const FaceControl* fc)
{
    if ( _views.count(fc) > 0)
    {
        removeActors(fc);
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
