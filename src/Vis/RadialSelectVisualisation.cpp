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
#include <FaceView.h>
#include <VtkTools.h>
#include <algorithm>
#include <cassert>
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Vis::LoopsView;
using FaceTools::LandmarkSet;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::FaceControlSet;
using FaceTools::FaceModelViewer;
using FaceTools::Action::ActionVisualise;

const double RadialSelectVisualisation::MIN_RADIUS = 7.0;


RadialSelectVisualisation::RadialSelectVisualisation( const QString& dname, const QIcon& icon)
    : BaseVisualisation(dname, icon), _radius(50)
{
}   // end ctor


RadialSelectVisualisation::~RadialSelectVisualisation()
{
    while (!_lviews.empty())
        purge( _lviews.begin()->first);
}   // end dtor


bool RadialSelectVisualisation::belongs( const vtkProp* prop, const FaceControl* fc) const
{   // Only check the centre reticule - outer ring is ignored.
    return (_sviews.count(fc) > 0) && (_sviews.at(fc)->prop() == prop);
}   // end belongs


bool RadialSelectVisualisation::isAvailable( const FaceControl* fc, const QPoint* mc) const
{
    assert(fc); // Available if at least one landmark OR a point is given and it's on the face.
    return !fc->data()->landmarks()->empty() || ( mc && fc->view()->isPointOnFace( *mc));
}   // end isAvailable


// private
void RadialSelectVisualisation::resetRegionSelector( const FaceModel* fm, const cv::Vec3f& cpos)
{
    _rselectors[fm] = RFeatures::ObjModelRegionSelector::create( fm->info()->cmodel(), cpos, fm->kdtree()->find(cpos));
    _rselectors.at(fm)->setRadius(_radius);
}   // end resetRegionSelector


// private
void RadialSelectVisualisation::createActors( const FaceControl* fc)
{
    const FaceModel* fm = fc->data();
    assert(_lviews.count(fc) == 0);
    _lviews[fc] = new LoopsView( 3.0f);
    _sviews[fc] = new SphereView( _rselectors.at(fm)->centre(), MIN_RADIUS, true/*pickable*/);
    _sviews[fc]->setResolution(101);
    setHighlighted( fc, false);
}   // end createActors


bool RadialSelectVisualisation::apply( const FaceControl* fc, const QPoint* mc)
{
    assert(fc);
    const FaceModel* fm = fc->data();
    bool applied = _rselectors.count(fm) > 0;
    cv::Vec3f cpos = centre(fm);
    if ( mc)
        applied = fc->view()->pointToFace( *mc, cpos);

    // If mouse coord didn't map to a face point, use the landmarks.
    if ( !applied)
    {
        // The preferred position to initially place the region is on the nose tip,
        // but if this landmark isn't present, use any landmark.
        const FaceTools::LandmarkSet::Ptr lmks = fm->landmarks();
        if ( lmks->has( FaceTools::Landmarks::NASAL_TIP))
        {
            cpos = lmks->pos( FaceTools::Landmarks::NASAL_TIP);
            applied = true;
        }   // end if
        else if ( !lmks->empty())
        {
            cpos = lmks->pos( *lmks->ids().begin());
            applied = true;
        }   // end else if
    }   // end if

    if ( applied)
    {
        if ( _rselectors.count(fm) == 0)
            resetRegionSelector( fm, cpos);

        if ( _lviews.count(fc) == 0)
        {
            createActors( fc);
            setCentre( fm, cpos);
        }   // end if
    }   // end if

    return applied;
}   // end apply


// private
void RadialSelectVisualisation::updateActors( const FaceModel* fm)
{
    assert(_rselectors.count(fm) > 0);

    // Get the current vertices forming the selected boundary loop.
    std::list<int> lvidxs;    // Get the vertices making the line
    const size_t nverts = _rselectors.at(fm)->getBoundary( lvidxs);
    std::list<cv::Vec3f> line;    // Convert to values
    const RFeatures::ObjModel* cmodel = fm->info()->cmodel();
    std::for_each( std::begin(lvidxs), std::end(lvidxs), [&](int v){ line.push_back(cmodel->vtx(v));});

    // DEBUG
    std::vector<cv::Vec3f> pts;
    const IntSet* vidxs = _rselectors.at(fm)->getBoundary();
    std::for_each( std::begin(*vidxs), std::end(*vidxs), [&](int v){ pts.push_back(cmodel->vtx(v));});

    const cv::Vec3f& cpos = centre(fm);

    // Reset for the associated FaceControls
    for ( const FaceControl* fc : fm->faceControls())
    {
        if ( _lviews.count(fc) == 0)
            createActors(fc);

        bool visible = _sviews.at(fc)->visible();
        _sviews.at(fc)->setCentre( cpos);

        // Regenerate the actors for the boundary.
        _lviews.at(fc)->setVisible( false, fc->viewer());
        _lviews.at(fc)->deleteActors();
        if ( nverts >= 3)
        {
            _lviews.at(fc)->addLoop( line);
            _lviews.at(fc)->addPoints( pts); // DEBUG
        }   // end if
        _lviews.at(fc)->setVisible( visible, fc->viewer());
    }   // end for
}   // end updateActors


void RadialSelectVisualisation::addActors( const FaceControl* fc)
{
    if ( _sviews.count(fc) > 0)
    {
        _sviews.at(fc)->setVisible( true, fc->viewer());
        _lviews.at(fc)->setVisible( true, fc->viewer());
    }   // end if
}   // end addActors


void RadialSelectVisualisation::removeActors( const FaceControl* fc)
{
    if ( _sviews.count(fc) > 0)
    {
        _sviews.at(fc)->setVisible( false, fc->viewer());
        _lviews.at(fc)->setVisible( false, fc->viewer());
    }   // end if
}   // end removeActors


cv::Vec3f RadialSelectVisualisation::centre( const FaceModel* fm) const
{
    assert(fm);
    if ( _rselectors.count(fm) == 0)
        return cv::Vec3f(0,0,0);
    return _rselectors.at(fm)->centre();
}   // end centre


void RadialSelectVisualisation::setCentre( const FaceModel* fm, const cv::Vec3f& cpos)
{
    assert( _rselectors.count(fm) > 0);

    // If cpos is on the same model component that the selector is already calculated for,
    // just set the centre as normal. If it's on a different component though, reset it.
    int nvidx = fm->kdtree()->find(cpos);  // New vidx 
    int cvidx = fm->kdtree()->find(centre(fm));   // Current vidx

    size_t nverts = 0;
    const RFeatures::ObjModelComponentFinder& cfinder = fm->info()->components();
    if ( cfinder.componentVerticesFromVertex(nvidx) == cfinder.componentVerticesFromVertex(cvidx))
        nverts = _rselectors.at(fm)->setCentre(cpos);

    if ( nverts == 0)
    {
        std::cerr << "[INFO] FaceTools::Vis::RadialSelectVisualisation::setCentre: "
                  << nverts << " in region; resetting ObjModelRegionSelector." << std::endl;
        resetRegionSelector( fm, cpos);
    }   // end if

    updateActors(fm);
}   // end setCentre


double RadialSelectVisualisation::radius( const FaceModel* fm) const
{
    assert( _rselectors.count(fm) > 0);
    if ( _rselectors.count(fm) == 0)
        return 0.0;
    return _rselectors.at(fm)->radius();
}   // end radius


void RadialSelectVisualisation::setRadius( const FaceModel* fm, double nrad)
{
    assert( _rselectors.count(fm) > 0);
    if ( _rselectors.count(fm) > 0)
    {
        nrad = std::max( nrad, MIN_RADIUS);
        _rselectors.at(fm)->setRadius(nrad);
        _radius = _rselectors.at(fm)->radius();
        updateActors(fm);
    }   // end if
}   // end setRadius


void RadialSelectVisualisation::setPickable( const FaceControl* fc, bool v)
{
    if ( _sviews.count(fc) > 0)
        _sviews.at(fc)->setPickable(v);
}   // end setPickable


bool RadialSelectVisualisation::setHighlighted( const FaceControl* fc, bool hval)
{
    if ( _lviews.count(fc) == 0)
        return false;

    cv::Vec3f colour(0.3f, 0.0f, 0.9f); // non-highlight colour
    if ( hval)
        colour = cv::Vec3f( 0.9f, 0.3f, 0.0f);

    bool setcolour = false;
    if ( colour != _lviews[fc]->colour())
    {
        _sviews[fc]->setColour( colour[0], colour[1], colour[2]);
        _sviews[fc]->setOpacity(0.5);
        _lviews[fc]->setColour( colour);
        setcolour = true;
    }   // end if
    return setcolour;
}   // end setHighlighted


// protected
void RadialSelectVisualisation::pokeTransform( const FaceControl* fc, const vtkMatrix4x4* vm)
{
    if (_lviews.count(fc) > 0)
    {
        _lviews.at(fc)->pokeTransform(vm);
        _sviews.at(fc)->pokeTransform(vm);
    }   // end if
}   // end pokeTransform


// protected
void RadialSelectVisualisation::fixTransform( const FaceControl* fc)
{
    if (_lviews.count(fc) > 0)
    {
        _lviews.at(fc)->fixTransform();
        _sviews.at(fc)->fixTransform();
        _rselectors.at(fc->data())->setCentre( _sviews.at(fc)->centre());   // Updating from a different (but agreeing) SphereView
    }   // end if
}   // end fixTransform


// protected
void RadialSelectVisualisation::purge( const FaceControl* fc)
{
    if (_lviews.count(fc) > 0)
    {
        delete _lviews.at(fc);
        _lviews.erase(fc);
        delete _sviews.at(fc);
        _sviews.erase(fc);
    }   // end if
}   // end purge


// protected
void RadialSelectVisualisation::purge( const FaceModel* fm) { _rselectors.erase(fm);}
