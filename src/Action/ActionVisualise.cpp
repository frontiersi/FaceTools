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

#include <ActionVisualise.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceViewerSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionVisualise::ActionVisualise( BaseVisualisation* vint)
    : FaceAction( vint->getDisplayName(), vint->getIcon()), _vis(vint)
{
    assert(vint);
    setCheckable( true, false);
    setVisible(_vis->isVisible());

    // Purge events
    ChangeEventSet pces;
    pces.insert(GEOMETRY_CHANGE);
    _vis->addPurgeEvents(pces);
    std::for_each( std::begin(pces), std::end(pces), [this](auto c){ this->addPurgeOn(c);});
}   // end ctor


bool ActionVisualise::testReady( const FaceControl* fc)
{
    return testChecked(fc) || (_vis->isAvailable( fc->data()) && (!_vis->isExclusive() || !_vis->isApplied(fc)));
}   // end testReady


void ActionVisualise::tellReady( FaceControl* fc, bool v)
{
    if ( _vis->applyOnReady())
    {
        setChecked(v);
        toggleVis(fc, nullptr);
    }   // end if
}   // end tellReady


bool ActionVisualise::testEnabled( const QPoint* mc) const
{
    FaceControl* fc = ready1();
    return fc && (testChecked(fc) || _vis->isAvailable( fc, mc) || (!_vis->singleModel() && !_vis->singleView()));
}   // end testEnabled


bool ActionVisualise::testChecked( const FaceControl* fc) const { return fc && _vis->isApplied(fc);}


// private
void ActionVisualise::toggleVis( FaceControl* fc, const QPoint* mc)
{
    FaceModel* fm = fc->data();
    fm->lockForRead();
    if ( isChecked() && _vis->isAvailable(fc->data()))
        fc->view()->apply( _vis, mc);
    else
        fc->view()->remove( _vis);
    setChecked( fc->view()->isApplied( _vis));
    fm->unlock();
}   // end toggleVis


bool ActionVisualise::doAction( FaceControlSet& fcs, const QPoint& mc)
{
    assert(fcs.size() == 1);
    FaceControl* fc = fcs.first();
    fcs.clear();

    if ( _vis->singleView() && _vis->singleModel())
    {
        std::cerr << "[WARNING] FaceTools::ActionVisualise::doAction: Visualisation " << debugActionName()
                  << " has both singleModel() and singleView() returning true which is not allowed!" << std::endl;
        assert(false);
    }   // end if

    if ( _vis->singleModel())
    {
        // Apply to all views corresponding to the selected model. Apply first to the selected FaceControl since
        // the visualisation may depend on the mouse coordinates for the viewer of the selected FaceControl to
        // create internal data structures associated with fc->data() (which is shared ) so that these shared
        // data are available for the subsequent FaceControl views.
        fcs.insert( fc->data()); // Ensure all present.
    }   // end if
    else if ( !_vis->singleView())
    {   // Apply to all FaceControls in the same viewer as the selected FaceControl.
        fcs = fc->viewer()->attached();
    }   // end else if

    toggleVis(fc, &mc); // Apply first to the selected FaceControl.
    fc->viewer()->updateRender();
    fcs.erase(fc);      // Don't apply the visualisation to the selected FaceControl again in the below loop
    std::for_each( std::begin(fcs), std::end(fcs), [&](auto f){ this->toggleVis(f, &mc); f->viewer()->updateRender();});  // Apply to all the others

    fcs.insert(fc); // Ensure the selected FaceControl is in the set on return.

    return true;
}   // end doAction


void ActionVisualise::purge( const FaceModel* fm)
{
    const FaceControlSet& fcs = fm->faceControls();
    // Purge the visualisation, but don't do this through the FaceView since that de-applies the visualisation.
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ _vis->removeActors(fc); _vis->purge(fc);});
    _vis->purge(fm);
}   // end purge


void ActionVisualise::clean( const FaceModel* fm)
{
    const FaceControlSet& fcs = fm->faceControls();
    // Remove through the FaceView since this removes the visualisation layer.
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ fc->view()->remove( _vis);});
    _vis->purge(fm);
}   // end clean
