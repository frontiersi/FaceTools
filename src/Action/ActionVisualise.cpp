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
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::ActionVisualise;
using FaceTools::Action::FaceAction;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ChangeEvent;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionVisualise::ActionVisualise( BaseVisualisation* vint, bool d)
    : FaceAction( vint->getDisplayName(), vint->getIcon(), true/*disable before other actions*/),
      _vint(vint), _isdefault(d)
{
    assert(vint);
    // Non-exclusive visualisations should not be the default!
    if ( _isdefault && !_vint->isExclusive())
        std::cerr << "[ERROR] FaceTools::Action::ActionVisualise: Making non-exclusive visualisation default!" << std::endl;
    assert( !_isdefault || _vint->isExclusive());
    setCheckable( !_vint->isExclusive(), false);    // Non-exclusive visualisations are checkable
    if ( vint->respondData())
        addRespondTo( DATA_CHANGE);
    if ( vint->respondCalc())
        addRespondTo( CALC_CHANGE);
    addRespondTo( VIEW_CHANGE);
    addChangeTo( VIEW_CHANGE);
}   // end ctor


bool ActionVisualise::testReady( FaceControl* fc)
{
    const std::string dname = debugActionName();
    const bool avail = _vint->isAvailable(fc->data());
    // If no visualisations have yet been applied to the face view, this action is the default
    // visualisation, and the visualisation is available for the model, then apply it.
    if ( fc->view()->visualisations().empty() && _isdefault && avail) // Set default initial visualisation
    {
        std::cerr << " + Applying " << dname << " (default visualisation)" << std::endl;
        fc->view()->rebuild();
        fc->view()->apply( _vint);     // Applies visualisation post-processing and sets in viewer.
    }   // end if

    const bool applied = _vint->isApplied(fc);
    if ( avail && applied)
    { 
        _vint->onSelected(fc);
        fc->viewer()->updateRender();
    }   // end if

    // Return ready only if visualisation available and is not currently set on the FaceControl,
    // unless this is a checkable visualisation (in which case running this action again when
    // applied, removes the visualisation).
    return avail && (isCheckable() || !applied);
}   // end testReady


bool ActionVisualise::testChecked( FaceControl* fc) { return _vint->isApplied(fc);}


bool ActionVisualise::doAction( FaceControlSet& s)
{
    const std::string dname = debugActionName();
    // Apply visualisation to each FaceControl. Default doAfterAction calls updateRender on viewers.
    if ( !isCheckable() || isChecked())
    {
        std::for_each( std::begin(s), std::end(s), [=]( auto fc)
                {
                    if ( !_vint->isApplied(fc))
                    {
                        std::cerr << " + Applying " << dname << " visualisation" << std::endl;
                        fc->view()->apply( _vint);
                    }   // end if
                });
    }   // end if
    else
    {
        std::for_each( std::begin(s), std::end(s), [=]( auto fc)
                {
                    if ( _vint->isApplied(fc))
                    {
                        std::cerr << " - Removing " << dname << " visualisation" << std::endl;
                        fc->view()->remove( _vint);
                    }   // end if
                });
    }   // end else
    return true;
}   // end doAction


void ActionVisualise::respondTo( const FaceAction* saction, const ChangeEventSet* ces, FaceControl* fc)
{
    const bool applied = _vint->isApplied(fc);

    // Call respondTo on visualisation for changes it cares about.
    if ( (_vint->respondData() && ces->count(DATA_CHANGE) > 0) || (_vint->respondCalc() && ces->count(CALC_CHANGE) > 0))
    {
        if ( applied)
            _vint->removeActors(fc);
        _vint->respondTo( saction, fc);
        if ( applied)
            _vint->addActors(fc);
    }   // end if

    FaceAction::respondTo( saction, ces, fc);    // Forwards through to testReady(fc)
    if ( applied)
        fc->viewer()->updateRender();
}   // end respondTo


void ActionVisualise::purge( const FaceControl* fc)
{
    assert(fc);
    assert(fc->viewer());
    fc->view()->remove(_vint);
    _vint->purge(fc);    // Ditch any cached visualisation specific stuff (legends etc).
}   // end purge
