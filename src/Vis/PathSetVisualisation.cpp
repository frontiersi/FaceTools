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

#include <PathSetVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::Vis::FV;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;
using FaceTools::Path;


// static
std::string PathSetVisualisation::CAPTION_LENGTH_METRIC("mm");


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
        purge( const_cast<FV*>(_views.begin()->first));
}   // end dtor


bool PathSetVisualisation::isAvailable( const FM* fm) const
{
    assert(fm);
    return !fm->paths()->empty();
}   // end isAvailable


bool PathSetVisualisation::belongs( const vtkProp* prop, const FV* fv) const
{
    return pathHandle( fv, prop) != nullptr;
}   // end belongs


void PathSetVisualisation::apply( FV* fv, const QPoint*)
{
    if ( !hasView(fv))
        _views[fv] = new PathSetView( fv->data()->paths());
    FMV* viewer = fv->viewer();
    _views.at(fv)->setVisible( true, viewer);
    viewer->add( _caption);
    viewer->add( _text);
}   // end apply


void PathSetVisualisation::remove( FV* fv)
{
    if ( hasView(fv))
    {
        FMV* viewer = fv->viewer();
        _views.at(fv)->setVisible( false, viewer);
        viewer->remove( _caption);
        viewer->remove( _text);
    }   // end if
}   // end remove


// public
void PathSetVisualisation::addPath( const FM* fm, int pathId)
{
    for ( FV* fv : fm->fvs())
    {
        if ( !hasView(fv))
            apply(fv,nullptr);
        else
            _views.at(fv)->updatePath(pathId);
        _views.at(fv)->showPath( true, pathId);
    }   // end for
}   // end addPath


// public
void PathSetVisualisation::updatePath( const FM* fm, int pathId)
{
    for ( FV* fv : fm->fvs())
        if ( hasView(fv))
            _views.at(fv)->updatePath( pathId);
}   // end updatePath


// public
void PathSetVisualisation::refresh( const FM* fm)
{
    assert(fm);
    for ( FV* fv : fm->fvs())
    {
        if ( !hasView(fv))  // Ensure the PathSetView is present
            apply(fv,nullptr);
        else
            _views.at(fv)->refresh();
    }   // end for
}   // end refresh


// public
void PathSetVisualisation::setCaptions( const std::string& pname, double elen, double psum, int xpos, int ypos)
{
    // Set the text contents for the label and the caption
    std::ostringstream oss0, oss1, oss2;
    if ( !pname.empty())
        oss0 << pname << std::endl;
    oss1 << "Caliper: " << std::setw(5) << std::fixed << std::setprecision(1) << elen << " " << CAPTION_LENGTH_METRIC << std::endl;
    oss2 << "Surface: " << std::setw(5) << std::fixed << std::setprecision(1) << psum << " " << CAPTION_LENGTH_METRIC << std::endl;
    _text->SetInput( (oss0.str() + oss1.str() + oss2.str()).c_str());
    _text->SetDisplayPosition( xpos, ypos);

    std::ostringstream caposs;
    caposs << std::fixed << std::setprecision(1) << elen << " " << CAPTION_LENGTH_METRIC;
    _caption->SetCaption( caposs.str().c_str());
}   // end setCaptions


// public
void PathSetVisualisation::setCaptionAttachPoint( const cv::Vec3f& av)
{
    double attachPoint[3] = { av[0], av[1], av[2]};
    _caption->SetAttachmentPoint( attachPoint);
}   // end setCaptionAttachPoint


const PathView::Handle* PathSetVisualisation::pathHandle( const FV* fv, const vtkProp* prop) const
{
    PathView::Handle *h = nullptr;
    if ( hasView(fv))
        h = _views.at(fv)->handle( prop);
    return h;
}   // end pathHandle


const PathView::Handle* PathSetVisualisation::pathHandle0( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( hasView(fv))
    {
        PathView* pv = _views.at(fv)->pathView(pid);
        if ( pv)
            h = pv->handle0();
    }   // end if
    return h;
}   // end pathHandle0


const PathView::Handle* PathSetVisualisation::pathHandle1( const FV* fv, int pid) const
{
    PathView::Handle* h = nullptr;
    if ( hasView(fv))
    {
        PathView* pv = _views.at(fv)->pathView(pid);
        if ( pv)
            h = pv->handle1();
    }   // end if
    return h;
}   // end pathHandle1

// public
void PathSetVisualisation::setCaptionsVisible( bool v)
{
    _caption->SetVisibility( v);
    _text->SetVisibility( v);
}   // end setCaptionsVisible


// protected
void PathSetVisualisation::pokeTransform( const FV* fv, const vtkMatrix4x4* m)
{
    if ( hasView(fv))
        _views.at(fv)->pokeTransform(m);
}   // end pokeTransform


// protected
void PathSetVisualisation::fixTransform( const FV* fv)
{
    if ( hasView(fv))
        _views.at(fv)->fixTransform();
}   // end fixTransform


// protected
void PathSetVisualisation::purge( FV* fv)
{
    if ( hasView(fv))
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge


// private
bool PathSetVisualisation::hasView( const FV* fv) const { return _views.count(fv) > 0;}
