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

#include <OutlinesVisualisation.h>
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::OutlinesVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::Vis::FV;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::ModelViewer;
using FaceTools::FVS;


OutlinesVisualisation::OutlinesVisualisation( const QString& dname, const QIcon& icon, const QKeySequence& keys)
    : BaseVisualisation(dname, icon, keys)
{
}   // end ctor


OutlinesVisualisation::~OutlinesVisualisation()
{
    while (!_views.empty())
        purge( const_cast<FV*>(_views.begin()->first));
}   // end dtor


void OutlinesVisualisation::apply( FV* fv, const QPoint*)
{
    if ( _views.count(fv) == 0)
    {
        LoopsView* ov = _views[fv] = new LoopsView( 5, 1.0/*red*/, 0.0/*green*/, 0.1/*blue*/);
        const RFeatures::ObjModelBoundaryFinder& boundaries = fv->data()->info()->boundaries();
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
    _views.at(fv)->setVisible( true, fv->viewer());
}   // end apply


void OutlinesVisualisation::clear( FV* fv)
{
    if (_views.count(fv) > 0)
        _views.at(fv)->setVisible( false, fv->viewer());
}   // end clear 


// protected
void OutlinesVisualisation::pokeTransform( const FV* fv, const vtkMatrix4x4* m)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->pokeTransform(m);
}   // end pokeTransform


// protected
void OutlinesVisualisation::fixTransform( const FV* fv)
{
    if ( _views.count(fv) > 0)
        _views.at(fv)->fixTransform();
}   // end fixTransform


// protected
void OutlinesVisualisation::purge( FV* fv)
{
    if (_views.count(fv) > 0)
    {
        _views.at(fv)->setVisible( false, fv->viewer());
        delete _views.at(fv);
        _views.erase(fv);
    }   // end if
}   // end purge
