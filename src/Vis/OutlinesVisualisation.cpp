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
#include <FaceControl.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::OutlinesVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::ModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


OutlinesVisualisation::OutlinesVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : BaseVisualisation(dname, icon, keys)
{
}   // end ctor


OutlinesVisualisation::~OutlinesVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool OutlinesVisualisation::apply( const FaceControl* fc, const QPoint*)
{
    if ( _views.count(fc) == 0)
    {
        LoopsView* ov = _views[fc] = new LoopsView( 5, 1.0/*red*/, 0.0/*green*/, 0.1/*blue*/);
        const RFeatures::ObjModelBoundaryFinder& boundaries = fc->data()->info()->boundaries();
        const int nbs = (int)boundaries.size();
        assert(nbs > 0);
        const RFeatures::ObjModel* model = boundaries.model();
        for ( int i = 0; i < nbs; ++i)
        {
            // Get the vertex positions for boundary i
            const std::list<int>& loop = boundaries.boundary(i);
            std::list<cv::Vec3f> line;
            std::for_each( std::begin(loop), std::end(loop), [&](int v){ line.push_back(model->vtx(v));});
            ov->addLoop(line);  // Add actor
        }   // end for
    }   // end if
    return true;
}   // end apply


void OutlinesVisualisation::addActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void OutlinesVisualisation::removeActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
        _views.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


// protected
void OutlinesVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* m)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->pokeTransform(m);
}   // end pokeTransform


// protected
void OutlinesVisualisation::fixTransform( const FaceControl* fc)
{
    if ( _views.count(fc) > 0)
        _views.at(fc)->fixTransform();
}   // end fixTransform


// protected
void OutlinesVisualisation::purge( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        removeActors(fc);
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
