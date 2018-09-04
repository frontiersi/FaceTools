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

#include <ActionRadialSelect.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <FaceModelViewer.h>
#include <LandmarkSet.h>
#include <cassert>
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::EventSet;
using FaceTools::Action::ActionVisualise;
using FaceTools::Vis::LoopSelectVisualisation;
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


const double ActionRadialSelect::MIN_RADIUS = 7.0;


ActionRadialSelect::ActionRadialSelect( const QString& dn, const QIcon& ico, MEEI* meei, QStatusBar* sbar)
    : ActionVisualise( _vis = new LoopSelectVisualisation( dn, ico)),
      _interactor( new RadialSelectInteractor( meei, _vis, sbar)),
      _radius(50)
{
    // Since this is an ActionVisualise, GEOMETRY_CHANGE events will cause everything to be purged.
    // In the case of an AFFINE_CHANGE though, we only need to adjust the centre of the region selector
    // since the boundary is defined in terms of vertex indices.
    connect( meei, &MEEI::onEnterModel, [=](auto fv){ this->testSetEnabled( &meei->viewer()->mouseCoords());});
    connect( meei, &MEEI::onLeaveModel, [this](auto fv){ this->testSetEnabled( nullptr);});

    connect( _interactor, &RadialSelectInteractor::onIncreaseRadius, this, &ActionRadialSelect::doOnIncreaseRadius);
    connect( _interactor, &RadialSelectInteractor::onDecreaseRadius, this, &ActionRadialSelect::doOnDecreaseRadius);
    connect( _interactor, &RadialSelectInteractor::onSetCentre, this, &ActionRadialSelect::doOnSetCentre);

    // Upon a change in model geometry, process this action setting check state to false because the region
    // selector will no longer be valid (or even present since ActionVisualise mandates a prior call to
    // purge on changes to geometry which will remove the corresponding region selector from the hash table).
    setRespondToEvent( GEOMETRY_CHANGE, false);
}   // end ctor


ActionRadialSelect::~ActionRadialSelect()
{
    delete _interactor;
    delete _vis;
}   // end dtor


// public
double ActionRadialSelect::radius( const FM* fm) const
{
    assert(_rsels.count(fm) > 0);
    return _rsels.at(fm)->radius();
}   // end radius


// public
cv::Vec3f ActionRadialSelect::centre( const FM* fm) const
{
    assert(_rsels.count(fm) > 0);
    fm->lockForRead();
    cv::Vec3f v = _rsels.at(fm)->centre( fm->info()->cmodel());
    fm->unlock();
    return v;
}   // end centre


// public
void ActionRadialSelect::selectedFaces( const FM* fm, IntSet& fs) const
{
    assert(_rsels.count(fm) > 0);
    fm->lockForRead();
    _rsels.at(fm)->selectedFaces( fm->info()->cmodel(), fs);
    fm->unlock();
}   // end selectedFaces


bool ActionRadialSelect::doAction( FVS& fvs, const QPoint& mc)
{
    assert(fvs.size() == 1);
    const FV* fv = fvs.first();
    const FM* fm = fv->data();
    cv::Vec3f cpos = _rsels.count(fm) > 0 ? centre(fm) : fm->findClosestSurfacePoint( fm->centre());

    // In the first case, select as the centre the point projected onto the surface my the
    // given 2D point (mouse coords). In the second instance, use the NASAL_TIP landmark
    // if available, otherwise just use the point on the surface closest to the model's centre.
    if ( !fv->projectToSurface( mc, cpos))
    {
        fm->lockForRead();
        const LandmarkSet::Ptr lmks = fm->landmarks();
        if ( lmks->has( FaceTools::Landmarks::NASAL_TIP))
            cpos = lmks->pos( FaceTools::Landmarks::NASAL_TIP);
        fm->unlock();
    }   // end if

    if ( _rsels.count(fm) == 0)
        makeRegionSelector( fm, cpos);

    bool actioned = ActionVisualise::doAction( fvs, mc);
    if ( isChecked())
        doOnSetCentre( fv, cpos);
    return actioned;
}   // end doAction


void ActionRadialSelect::doAfterAction( EventSet& cs, const FVS& fvs, bool v)
{
    ActionVisualise::doAfterAction( cs, fvs, v);
    _interactor->setEnabled( isChecked());
}   // end doAfterAction


void ActionRadialSelect::purge( const FM* fm)
{
    ActionVisualise::purge(fm);
    _rsels.erase(fm);
}   // end purge


// private
void ActionRadialSelect::makeRegionSelector( const FM* fm, const cv::Vec3f& cpos)
{
    fm->lockForRead();
    const int sv = fm->kdtree()->find( cpos);
    _rsels[fm] = RFeatures::ObjModelRegionSelector::create( fm->info()->cmodel(), sv);
    _rsels[fm]->setRadius( fm->info()->cmodel(), _radius);
    fm->unlock();
}   // end makeRegionSelector


// private slot
void ActionRadialSelect::doOnIncreaseRadius( const FV* fv)
{
    const FM* fm = fv->data();
    setRadius( fm, _rsels.at(fm)->radius() + 1);
}   // end doOnIncreaseRadius


// private slot
void ActionRadialSelect::doOnDecreaseRadius( const FV* fv)
{
    const FM* fm = fv->data();
    setRadius( fm, std::max( _rsels.at(fm)->radius() - 1, MIN_RADIUS));
}   // end doOnDecreaseRadius


// private
void ActionRadialSelect::setRadius( const FM* fm, double nrad)
{
    _radius = nrad;
    fm->lockForRead();
    _rsels.at(fm)->setRadius( fm->info()->cmodel(), nrad);    // Change the selected radius
    fm->unlock();
    updateVis(fm);
}   // end setRadius


// private slot
void ActionRadialSelect::doOnSetCentre( const FV* fv, const cv::Vec3f& v)
{
    const FM* fm = fv->data();
    fm->lockForRead();
    int cvidx = fm->kdtree()->find(v);
    cv::Vec3f offset = v - fm->info()->cmodel()->vtx(cvidx);    // Required offset from the vertex
    fm->unlock();
    _rsels.at(fm)->setCentre( fm->info()->cmodel(), cvidx, offset);  // Offset is stored in local coordinate frame (wrt vertex).
    updateVis(fm);
}   // end doOnSetCentre


// private
void ActionRadialSelect::updateVis( const FM* fm)
{
    assert(_rsels.count(fm) > 0);
    if ( _rsels.count(fm) == 0)
        return;

    // Get the boundary as a vector of vertices
    const IntSet* vidxs = _rsels.at(fm)->boundary();
    std::vector<cv::Vec3f> pts;
    fm->lockForRead();
    const RFeatures::ObjModel* cmodel = fm->info()->cmodel();
    std::for_each( std::begin(*vidxs), std::end(*vidxs), [&](int v){ pts.push_back(cmodel->vtx(v));});

    // Get the centre vertex
    cv::Vec3f v = _rsels.at(fm)->centre( cmodel);

    for ( const FV* fv : fm->fvs())
    {
        _vis->setReticule( fv, v);
        _vis->setPoints( fv, pts);
    }   // end for

    fm->updateRenderers();
    fm->unlock();
}   // end updateVis
