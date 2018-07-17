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
    return _vis->isAvailable(fc->data()) && (!_vis->isExclusive() || !_vis->isApplied(fc));
}   // end testReady


void ActionVisualise::tellReady( FaceControl* fc, bool v)
{
    if ( _vis->applyOnReady())
    {
        setChecked(v);
        toggleVis(fc);
    }   // end if
}   // end tellReady


bool ActionVisualise::testEnabled() const
{
    return readyCount() == 1 || (!_vis->singleModel() && !_vis->singleView() && readyCount() > 0);
}   // end testEnabled


bool ActionVisualise::testChecked( FaceControl* fc)
{
    return _vis->isApplied(fc);
}   // end testChecked


// private
void ActionVisualise::toggleVis( FaceControl* fc)
{
    FaceModel* fm = fc->data();
    fm->lockForRead();
    if ( isChecked() && _vis->isAvailable(fc->data()))
        fc->view()->apply( _vis);
    else
    {
        fc->view()->remove( _vis);
        setChecked(false);
    }   // end else
    fm->unlock();
}   // end toggleVis


bool ActionVisualise::doAction( FaceControlSet& fcs)
{
    if ( _vis->singleModel() || _vis->singleView())
    {
        assert(fcs.size() == 1);
        FaceControl* fc = fcs.first();
        assert(fc);
        // Apply to single view only
        if ( _vis->singleView())
        {
            toggleVis(fc);
            fcs.insert(fc);
        }   // end if
        else
        {
            FaceModel* fm = fc->data();
            assert(fm);
            // Apply to all views corresponding to the selected model
            for ( FaceControl* f : fm->faceControls())
            {
                toggleVis(f);
                fcs.insert(f);
            }   // end for
        }   // end else
    }   // end if
    else
    {
        // Apply to all FaceControls that the visualisation is available for in all directly accessible (selected) viewers.
        FaceViewerSet fvs = fcs.directViewers();  // Don't use fcs.viewers() because that will return all viewers for all data.
        for ( const FaceModelViewer* v : fvs)
        {
            for ( FaceControl* fc : v->attached())
            {
                toggleVis(fc);
                fcs.insert(fc);
            }   // end for
        }   // end for
    }   // end else

    return true;
}   // end doAction


void ActionVisualise::purge( const FaceModel* fm)
{
    for ( FaceControl* fc : fm->faceControls())
        _vis->purge(fc);
}   // end purge
