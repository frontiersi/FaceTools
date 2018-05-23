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

#include <BoundingVisualisation.h>
#include <FaceShapeLandmarks2DDetector.h>   // FaceTools::Landmarks
#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <CameraParams.h>   // RFeatures
#include <LandmarkSet.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <VtkTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::BoundingVisualisation;
using FaceTools::Vis::SphereView;
using FaceTools::LandmarkSet;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Action::ActionVisualise;


BoundingVisualisation::BoundingVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon)
{
}   // end ctor


BoundingVisualisation::~BoundingVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool BoundingVisualisation::isAvailable( const FaceModel* fm) const { return fm->landmarks().has( FaceTools::Landmarks::NASAL_TIP);}


void BoundingVisualisation::apply( const FaceControl* fc)
{
    if ( _views.count(fc) == 0)
    {
        _views[fc] = new SphereView( fc->data()->landmarks().pos( FaceTools::Landmarks::NASAL_TIP), 40, false);
        _views[fc]->setResolution(201);
        _views[fc]->setOpacity(0.4);
    }   // end if
}   // end apply


void BoundingVisualisation::addActors( const FaceControl* fc)
{
    _views.at(fc)->setVisible( true, fc->viewer());
}   // end addActors


void BoundingVisualisation::removeActors( const FaceControl* fc)
{
    _views.at(fc)->setVisible( false, fc->viewer());
}   // end removeActors


cv::Vec3f BoundingVisualisation::centre( const FaceControl* fc) const
{
    assert(_views.count(fc) > 0);
    return _views.at(fc)->centre();
}   // end centre


void BoundingVisualisation::setCentre( const FaceControl* fc, const cv::Vec3f& v)
{
    assert(_views.count(fc) > 0);
    _views.at(fc)->setCentre(v);
    fc->viewer()->updateRender();
}   // end setCentre


double BoundingVisualisation::radius( const FaceControl* fc) const
{
    assert(_views.count(fc) > 0);
    return _views.at(fc)->radius();
}   // end radius


void BoundingVisualisation::setRadius( const FaceControl* fc, double nrad)
{
    assert(_views.count(fc) > 0);
    _views.at(fc)->setRadius(nrad);
    fc->viewer()->updateRender();
}   // end setRadius


// protected
void BoundingVisualisation::transform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    cv::Vec3f npos = centre(fc);
    RFeatures::Transformer mover( RVTK::toCV(vm));
    mover.transform( npos);   // Transform position
    setCentre( fc, npos);   // Set back
}   // end transform


// protected
void BoundingVisualisation::purge( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        delete _views.at(fc);
        _views.erase(fc);
    }   // end if
}   // end purge
