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

#include <Vis/PathSetView.h>
#include <FaceTools.h>
#include <FaceModel.h>
#include <vtkTextProperty.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::PathSetView;
using FaceTools::Vis::PathView;
using FaceTools::ModelViewer;
using FaceTools::PathSet;
using FaceTools::Path;
using ViewPair = std::pair<int, PathView*>;


PathSetView::PathSetView() : _viewer(nullptr)
{
    // The bottom right text.
    _text->GetTextProperty()->SetJustificationToRight();
    _text->GetTextProperty()->SetFontFamilyToCourier();
    _text->GetTextProperty()->SetFontSize(21);
    _text->GetTextProperty()->SetBackgroundOpacity(0.7);
    _text->SetPickable( false);
    _text->SetVisibility(false);
}   // end ctor


PathSetView::~PathSetView()
{
    std::for_each( std::begin(_views), std::end(_views), [](const ViewPair& p){ delete p.second;});
}   // end dtor


bool PathSetView::isVisible() const { return !_visible.empty();}


void PathSetView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer)
        _viewer->remove(_text);

    while ( isVisible())
        _showPath( false, *_visible.begin());

    _viewer = viewer;

    if ( enable && _viewer)
    {
        for ( const auto& p : _views)
            _showPath( true, p.first);
        _viewer->add(_text);
    }   // end if
}   // end setVisible


void PathSetView::_showPath( bool enable, int id)
{
    assert( _views.count(id) > 0);
    _views.at(id)->setVisible( enable, _viewer);
    _visible.erase(id);
    if ( enable)
        _visible.insert(id);
}   // end _showPath


void PathSetView::setText( const Path& path, int xpos, int ypos)
{
    const std::string lnunits = FaceTools::FM::LENGTH_UNITS.toStdString();
    // Set the text contents for the label and the caption
    std::ostringstream oss0, oss1, oss2;
    if ( !path.name.empty())
        oss0 << path.name << std::endl;
    oss1 << "Caliper: " << std::setw(5) << std::fixed << std::setprecision(1) << path.elen << " " << lnunits;
    oss2 << "\nSurface: ";
    if ( path.psum > 0.0)
        oss2 << std::setw(5) << std::fixed << std::setprecision(1) << path.psum << " " << lnunits;
    else
        oss2 << " N/A";
    _text->SetInput( (oss0.str() + oss1.str() + oss2.str()).c_str());
    _text->SetDisplayPosition( xpos, ypos);
}   // end setText


void PathSetView::setTextVisible( bool v) { _text->SetVisibility(v);}


PathView::Handle* PathSetView::handle( const vtkProp* prop) const
{
    return _handles.count(prop) > 0 ? _handles.at(prop) : nullptr;
}   // end handle


PathView* PathSetView::pathView( int id) const
{
    return _views.count(id) > 0 ?  _views.at(id) : nullptr;
}   // end pathView


void PathSetView::addPath( const Path& path, const vtkMatrix4x4* d)
{
    // The vertices of the given path are always in their transformed positions.
    // For visualisation, the vertices must be untransformed so that pokeTransform
    // can be used to keep the view position of the vertices synchronised when
    // dynamically adjusting the position of the model.
    const cv::Matx44d imat = RVTK::toCV(d).inv();
    std::list<cv::Vec3f> vtxs;
    for ( const cv::Vec3f& v : path.vtxs)
        vtxs.push_back( RFeatures::transform( imat, v));

    PathView* pview = _views[path.id] = new PathView( path.id, vtxs);
    pview->pokeTransform(d);

    // Map handle props to the handles themselves for fast lookup.
    _handles[pview->handle0()->prop()] = pview->handle0();
    _handles[pview->handle1()->prop()] = pview->handle1();
}   // end addPath


void PathSetView::movePath( const Path& path)
{
    assert( _views.count(path.id) > 0);
    _views.at(path.id)->update(path.vtxs);
}   // end movePath


void PathSetView::erasePath( int id)
{
    assert( _views.count(id) > 0);
    _showPath( false, id);
    PathView* pv = _views.at(id);
    _handles.erase(pv->handle0()->prop());
    _handles.erase(pv->handle1()->prop());
    _views.erase(id);
    delete pv;
}   // end erasePath


void PathSetView::sync( const PathSet& paths, const vtkMatrix4x4* d)
{
    bool isVis = isVisible();

    // First remove any entries from _views that aren't in paths
    IntSet rpids;
    for ( auto& p : _views)
    {
        if ( !paths.has( p.first))
            rpids.insert(p.first);
    }   // end for

    for ( int pid : rpids)
        erasePath( pid);

    // Then add any in paths not yet present or update what is there
    for ( int pid : paths.ids())
    {
        const Path& path = *paths.path(pid);
        if ( _views.count(pid) == 0)
            addPath( path, d);
        else
            movePath( path);
    }   // end for

    isVis = isVis && !_views.empty();
    if ( isVis)
        setVisible(true, _viewer);
}   // end sync


void PathSetView::pokeTransform( const vtkMatrix4x4* vm)
{
    // Update all positions
    for ( auto& p : _views)
        p.second->pokeTransform(vm);
}   // end pokeTransform


void PathSetView::updateTextColours()
{
    if ( _viewer)
    {
        const QColor bg = _viewer->backgroundColour();
        const QColor fg = chooseContrasting(bg);
        _text->GetTextProperty()->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
        _text->GetTextProperty()->SetColor( fg.redF(), fg.greenF(), fg.blueF());
        for ( auto& p : _views)
            p.second->updateColours();
    }   // end if
}   // end updateTextColours
