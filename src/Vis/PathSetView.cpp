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
    _caption->GetTextProperty()->SetJustificationToRight();
    _caption->GetTextProperty()->SetFontFamilyToCourier();
    _caption->GetTextProperty()->SetFontSize(21);
    _caption->GetTextProperty()->SetBackgroundOpacity(0.7);
    _caption->SetPickable( false);
    _caption->SetVisibility(false);
}   // end ctor


PathSetView::~PathSetView()
{
    std::for_each( std::begin(_views), std::end(_views), [](const ViewPair& p){ delete p.second;});
}   // end dtor


void PathSetView::setVisible( bool enable, ModelViewer *viewer)
{
    if ( _viewer)
        _viewer->remove(_caption);

    while ( !_visible.empty())
        _showPath( false, *_visible.begin());

    _viewer = viewer;

    if ( enable && _viewer)
    {
        for ( const auto& p : _views)
            _showPath( true, p.first);
        _viewer->add(_caption);
    }   // end if
}   // end setVisible


bool PathSetView::isVisible() const { return !_visible.empty();}


void PathSetView::_showPath( bool enable, int id)
{
    assert( _views.count(id) > 0);
    _views.at(id)->setVisible( enable, _viewer);
    _visible.erase(id);
    if ( enable)
        _visible.insert(id);
}   // end _showPath


namespace {
std::string appendValue( float v, const std::string &send="")
{
    std::ostringstream oss;
    oss << " " << std::setw(7) << std::fixed << std::setprecision(2) << v << " " << std::setw(4) << std::left << send;
    return oss.str();
}   // end appendValue
}   // end namespace


void PathSetView::setCaption( const Path& path, int xpos, int ypos, const Mat3f &iR)
{
    const std::string lnunits = FaceTools::FM::LENGTH_UNITS.toStdString();
    // Set the text contents for the label and the caption
    std::ostringstream oss0, oss1, oss2, oss3, oss4, oss5, oss6, oss7, oss8, oss9;
    if ( !path.name().empty())
        oss0 << std::left << std::setfill(' ') << std::setw(21) << path.name() << std::endl;

    if (path.validPath())
    {
        oss1 << "   Surface:" << appendValue( path.surfaceDistance(), lnunits);
        oss3 << "\n   Ratio:" << appendValue( path.surface2EuclideanRatio());
    }   // end if
    else
    {
        oss1 << "   Surface:  Not Found  ";
        oss3 << "\n   Ratio:     N/A     ";
    }   // end else

    const Vec3f avec = iR * path.deltaVector();
    oss2 << "\n  Direct:" << appendValue( path.euclideanDistance(), lnunits);
    oss4 << "\n  X Size:" << appendValue( fabsf(avec[0]), lnunits);
    oss5 << "\n  Y Size:" << appendValue( fabsf(avec[1]), lnunits);
    oss6 << "\n  Z Size:" << appendValue( fabsf(avec[2]), lnunits);
    oss7 << "\n   Depth:" << appendValue( path.depth(), lnunits);
    oss8 << "\n   Angle:" << appendValue( path.angleAtDepth(), "degs");
    oss9 << "\n    Area:" << appendValue( path.crossSectionalArea(), lnunits + "^2");
    _caption->SetInput( (oss0.str() + oss1.str() + oss2.str() + oss3.str() + oss4.str() + oss5.str() + oss6.str() + oss7.str() + oss8.str() + oss9.str()).c_str());
    _caption->SetDisplayPosition( xpos, ypos);
}   // end setCaption


void PathSetView::setCaptionVisible( bool v) { _caption->SetVisibility(v);}


PathView::Handle* PathSetView::handle( const vtkProp* prop) const
{
    return _handles.count(prop) > 0 ? _handles.at(prop) : nullptr;
}   // end handle


PathView* PathSetView::pathView( int id) const
{
    return _views.count(id) > 0 ?  _views.at(id) : nullptr;
}   // end pathView


void PathSetView::addPath( const Path& path)
{
    PathView* pview = _views[path.id()] = new PathView( path.id());
    // Map handle props to the handles themselves for fast lookup.
    _handles[pview->handle0()->prop()] = pview->handle0();
    _handles[pview->handle1()->prop()] = pview->handle1();
    _handles[pview->depthHandle()->prop()] = pview->depthHandle();
    updatePath( path);
}   // end addPath


void PathSetView::updatePath( const Path& path)
{
    assert( _views.count(path.id()) > 0);
    _views.at(path.id())->update( path);
}   // end updatePath


void PathSetView::erasePath( int id)
{
    assert( _views.count(id) > 0);
    _showPath( false, id);
    PathView* pv = _views.at(id);
    _handles.erase(pv->handle0()->prop());
    _handles.erase(pv->handle1()->prop());
    _handles.erase(pv->depthHandle()->prop());
    _views.erase(id);
    delete pv;
}   // end erasePath


void PathSetView::sync( const PathSet& paths)
{
    bool isVis = !_visible.empty();
    // First remove any entries from _views that aren't in paths
    IntSet rpids;
    for ( auto& p : _views)
        if ( !paths.has( p.first))
            rpids.insert(p.first);

    for ( int pid : rpids)
        erasePath( pid);

    // Then add any in paths not yet present or update what is there
    for ( int pid : paths.ids())
    {
        const Path& path = paths.path(pid);
        if ( _views.count(pid) == 0)
            addPath( path);
        else
            updatePath( path);
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
        _caption->GetTextProperty()->SetBackgroundColor( bg.redF(), bg.greenF(), bg.blueF());
        _caption->GetTextProperty()->SetColor( fg.redF(), fg.greenF(), fg.blueF());
        for ( auto& p : _views)
            p.second->updateColours();
    }   // end if
}   // end updateTextColours
