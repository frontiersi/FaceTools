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

#include <OutlinesVisualisation.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <OutlinesView.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::OutlinesVisualisation;
using FaceTools::Vis::OutlinesView;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using FaceTools::FaceModel;

namespace {
void updateRender( const std::unordered_map<const FaceControl*, OutlinesView*>& views)
{
    std::unordered_set<ModelViewer*> viewers;
    std::for_each( std::begin(views), std::end(views),    [&](auto f){ viewers.insert(f.first->viewer());});
    std::for_each( std::begin(viewers), std::end(viewers), [](auto v){ v->updateRender();});
}   // end updateRender
}   // end namespace


OutlinesVisualisation::OutlinesVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : BaseVisualisation(dname, icon, keys)
{
}   // end ctor


OutlinesVisualisation::~OutlinesVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


void OutlinesVisualisation::apply( const FaceControl* fc)
{
    if ( _views.count(fc) == 0)
        _views[fc] = new OutlinesView( fc->data()->info()->boundaries());
}   // end apply


void OutlinesVisualisation::addActors( const FaceControl* fc)
{
    _views.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void OutlinesVisualisation::removeActors( const FaceControl* fc)
{
    _views.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


// protected
void OutlinesVisualisation::respondTo( const FaceAction*, const FaceControl* fc)
{
    purge(fc);
    apply(fc);
}   // end respondTo


// protected
void OutlinesVisualisation::transform( const FaceControl* fc, const vtkMatrix4x4* m)
{
    assert(_views.count(fc) > 0);
    _views.at(fc)->transform(m);
}   // end transform


// protected
void OutlinesVisualisation::purge( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
