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
using FaceTools::Vis::BaseVisualisation;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionVisualise::ActionVisualise( BaseVisualisation* vint, bool d)
    : FaceAction( true/*disable before other actions*/), _vint(vint), _isdefault(d)
{
    assert(vint);
    addChangeTo(  VISUALISATION_CHANGED);
    addRespondTo( VISUALISATION_CHANGED);
    // Non-exclusive visualisations should not be the default!
    if ( _isdefault && !_vint->isExclusive())
        std::cerr << "[ERROR] FaceTools::Action::ActionVisualise: Making non-exclusive visualisation default!" << std::endl;
    assert( !_isdefault || _vint->isExclusive());
    setCheckable( !_vint->isExclusive(), false);    // Non-exclusive visualisations are checkable
    vint->setAction(this);  // Allows delegate to add its own ChangeEvents to this FaceAction.
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

    setChecked(applied);

    // Return ready only if visualisation available and is not currently set on the FaceControl,
    // unless this is a checkable visualisation (in which case running this action again when
    // applied, removes the visualisation).
    return avail && (isCheckable() || !applied);
}   // end testReady


bool ActionVisualise::doAction( FaceControlSet& s)
{
    // Apply visualisation to each FaceControl. Default doAfterAction calls updateRender on viewers.
    if ( !isCheckable() || isChecked())
    {
        std::cerr << " + Applying visualisation" << std::endl;
        std::for_each( std::begin(s), std::end(s), [=]( auto fc){ fc->view()->apply( _vint);});
    }   // end if
    else
    {
        std::cerr << " - Removing visualisation" << std::endl;
        std::for_each( std::begin(s), std::end(s), [=]( auto fc){ fc->view()->remove( _vint);});
    }   // end else
    return true;
}   // end doAction


void ActionVisualise::respondToChange( FaceControl* fc)
{
    _vint->respondTo(fc);
    FaceAction::respondToChange(fc);
    if ( _vint->isApplied(fc))
        fc->viewer()->updateRender();
}   // end respondToChange


void ActionVisualise::burn( const FaceControl* fc)
{
    assert(fc);
    assert(fc->viewer());
    fc->view()->remove(_vint);
    _vint->burn(fc);    // Ditch any cached visualisation specific stuff (legends etc).
}   // end burn
