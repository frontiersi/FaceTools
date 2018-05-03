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
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionVisualise::ActionVisualise( Vis::BaseVisualisation* vint, bool d)
    : FaceAction( true/*disable before other actions*/), _vint(vint), _isdefault(d)
{
    assert(vint);
    addChangeTo(  VISUALISATION_CHANGED);
    addRespondTo( VISUALISATION_CHANGED);
    addRespondTo( MODEL_GEOMETRY_CHANGED);
    vint->setAction(this);  // Allows delegate to add its own ChangeEvents to this FaceAction.
    // Non-exclusive visualisations should not be the default!
    if ( _isdefault && !_vint->isExclusive())
        std::cerr << "[ERROR] FaceTools::Action::ActionVisualise: Making non-exclusive visualisation default!" << std::endl;
    assert( !_isdefault || _vint->isExclusive());
}   // end ctor


bool ActionVisualise::testReady( FaceControl* fc)
{
    const bool avail = _vint->isAvailable(fc->data());
    // If no visualisations have yet been applied to the face view, this action is the default
    // visualisation, and the visualisation is available for the model, then apply it.
    if ( fc->view()->visualisations().empty() && _isdefault && avail) // Set default initial visualisation
    {
        fc->view()->rebuild();
        fc->view()->apply( _vint);     // Applies visualisation post-processing and sets in viewer.
    }   // end if

    if ( avail && fc->view()->visualisations().count(_vint) > 0)
    { 
        _vint->onSelected(fc);
        fc->viewer()->updateRender();
    }   // end if

    // Return ready only if visualisation available and is not currently set on the FaceControl
    return avail && fc->view()->visualisations().count( _vint) == 0;
}   // end testReady


bool ActionVisualise::doAction( FaceControlSet& s)
{
    // Apply visualisation to each FaceControl. Default doAfterAction calls updateRender on viewers.
    std::for_each( std::begin(s), std::end(s), [=]( auto fc){ fc->view()->apply( _vint);});
    return true;
}   // end doAction


void ActionVisualise::respondToChange( FaceControl* fc)
{
    if ( fc->view()->visualisations().count(_vint) > 0)  // Refresh visualisation if this one?
    {
        FaceAction* sending = qobject_cast<FaceAction*>(sender());
        assert(sending);
        // Only rebuild the visualisation model if the sending action changes the model's geometry.
        if ( sending->changeEvents().count( MODEL_GEOMETRY_CHANGED))
            fc->view()->rebuild();  // Also reapplies this visualisation
        _vint->respondTo(fc);
        fc->viewer()->updateRender();
    }   // end if
    FaceAction::respondToChange(fc);    // Test and set if this visualisation action should be enabled
}   // end respondToChange


void ActionVisualise::burn( const FaceControl* fc)
{
    assert(fc);
    assert(fc->viewer());
    fc->view()->remove(_vint);
    _vint->burn(fc);    // Ditch any cached visualisation specific stuff (legends etc).
}   // end burn
