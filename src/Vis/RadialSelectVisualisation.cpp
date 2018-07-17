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

#include <RadialSelectVisualisation.h>
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
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::LandmarkSet;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using FaceTools::FaceControlSet;
using FaceTools::FaceModel;
using FaceTools::Action::ActionVisualise;


RadialSelectVisualisation::RadialSelectVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon), _radius(50)
{
}   // end ctor


RadialSelectVisualisation::~RadialSelectVisualisation()
{
    while (!_views.empty())
        purge(_views.begin()->first);
}   // end dtor


bool RadialSelectVisualisation::isAvailable( const FaceModel* fm) const
{
    return !fm->landmarks()->empty();
}   // end isAvailable


void RadialSelectVisualisation::apply( const FaceControl* fc)
{
    const FaceModel* fm = fc->data();
    if ( _rselectors.count(fm) == 0)
    {
        const FaceTools::LandmarkSet::Ptr lmks = fc->data()->landmarks();
        // The preferred position to initially place the region is on the nose tip,
        // but if this landmark isn't present, use any landmark.
        cv::Vec3f cpos(0,0,0);
        if ( lmks->has( FaceTools::Landmarks::NASAL_TIP))
            cpos = lmks->pos( FaceTools::Landmarks::NASAL_TIP);
        else
            cpos = lmks->pos( *lmks->ids().begin());

        const int seedvtx = fm->kdtree()->find(cpos);
        _rselectors[fm] = RFeatures::ObjModelRegionSelector::create(fm->info()->cmodel(), cpos, seedvtx);
        _rselectors.at(fm)->setRadius(_radius);
    }   // end if

    if ( _views.count(fc) == 0)
    {
        _views[fc] = new LoopsView( 3.0f, 0.0f, 0.3f, 0.8f);
        _views2[fc] = new SphereView( _rselectors.at(fm)->centre(), 5, false);
        _views2[fc]->setResolution(201);
        _views2[fc]->setColour( 0.0, 0.3, 0.8);
        _views2[fc]->setOpacity(0.5);
        updateView(fc);
    }   // end if
}   // end apply


// private
void RadialSelectVisualisation::updateView( const FaceControl* fc)
{
    const FaceModel* fm = fc->data();
    assert(_rselectors.count(fm) > 0);

    // Get the current vertices forming the selected boundary loop.
    std::list<int> lvidxs;    // Get the vertices making the line
    _rselectors.at(fm)->getBoundary( lvidxs);
    std::list<cv::Vec3f> line;    // Convert to values
    const RFeatures::ObjModel* cmodel = fm->info()->cmodel();
    std::for_each( std::begin(lvidxs), std::end(lvidxs), [&](int v){ line.push_back(cmodel->vtx(v));});

    // DEBUG
    std::vector<cv::Vec3f> pts;
    const IntSet* vidxs = _rselectors.at(fm)->getBoundary();
    std::for_each( std::begin(*vidxs), std::end(*vidxs), [&](int v){ pts.push_back(cmodel->vtx(v));});

    assert(_views.count(fc) > 0);
    // Reset in the view for this FaceControl
    bool visible = _views.at(fc)->isVisible();
    _views.at(fc)->removeActors();
    _views.at(fc)->addLoop( line);
    _views.at(fc)->addPoints( pts); // DEBUG
    _views.at(fc)->setVisible( visible, fc->viewer());

    _views2.at(fc)->setCentre( _rselectors.at(fm)->centre());
    _views2.at(fc)->setVisible( visible, fc->viewer());
}   // end updateView


void RadialSelectVisualisation::addActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        _views.at(fc)->setVisible( true, fc->viewer());
        _views2.at(fc)->setVisible( true, fc->viewer());
    }   // end if
}   // end addActors


void RadialSelectVisualisation::removeActors( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        _views.at(fc)->setVisible( false, fc->viewer());
        _views2.at(fc)->setVisible( false, fc->viewer());
    }   // end if
}   // end removeActors


cv::Vec3f RadialSelectVisualisation::centre( const FaceModel* fm) const
{
    if ( _rselectors.count(fm) == 0)
        return cv::Vec3f(0,0,0);
    return _rselectors.at(fm)->centre();
}   // end centre


void RadialSelectVisualisation::setCentre( const FaceModel* fm, const cv::Vec3f& v)
{
    assert(_rselectors.count(fm) > 0);
    _rselectors.at(fm)->setCentre(v);
    const FaceControlSet& fcs = fm->faceControls();
    std::for_each( std::begin(fcs), std::end(fcs), [this](auto fc){ this->updateView(fc);});
}   // end setCentre


double RadialSelectVisualisation::radius( const FaceModel* fm) const
{
    if ( _rselectors.count(fm) == 0)
        return 0.0;
    return _rselectors.at(fm)->radius();
}   // end radius


void RadialSelectVisualisation::setRadius( const FaceModel* fm, double nrad)
{
    assert(_rselectors.count(fm) > 0);
    _rselectors.at(fm)->setRadius(nrad);
    _radius = nrad;
    const FaceControlSet& fcs = fm->faceControls();
    std::for_each( std::begin(fcs), std::end(fcs), [this](auto fc){ this->updateView(fc);});
}   // end setRadius


// protected
void RadialSelectVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    if (_views.count(fc) > 0)
    {
        _views.at(fc)->pokeTransform(vm);
        _views2.at(fc)->pokeTransform(vm);
    }   // end if
}   // end pokeTransform


// protected
void RadialSelectVisualisation::fixTransform( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        _views.at(fc)->fixTransform();
        _views2.at(fc)->fixTransform();
    }   // end if
}   // end fixTransform


// protected
void RadialSelectVisualisation::purge( const FaceControl* fc)
{
    if (_views.count(fc) > 0)
    {
        delete _views.at(fc);
        _views.erase(fc);
        delete _views2.at(fc);
        _views2.erase(fc);
    }   // end if

    // Check if we need to purge the region selector too.
    bool purgeRegionSelector = true;
    const FaceModel* fm = fc->data();
    for ( const FaceControl* f : fm->faceControls())
    {
        if ( _views.count(f) > 0)
        {
            purgeRegionSelector = false;
            break;
        }   // end if
    }   // end for

    if ( purgeRegionSelector)
        _rselectors.erase(fm);
}   // end purge
