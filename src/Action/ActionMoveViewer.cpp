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

#include <ActionMoveViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMoveViewer;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Vis::BaseVisualisation;


ActionMoveViewer::ActionMoveViewer( FaceModelViewer *tv, ModelSelector* s, FaceModelViewer *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _selector(s), _sviewer(sv)
{
}   // end ctor


bool ActionMoveViewer::testReady( const FaceControl* fc)
{
    bool allowed = true;
    if ( _sviewer != nullptr)
        allowed = _sviewer->isAttached(fc);
    return allowed;
}   // end testReady


// protected
bool ActionMoveViewer::doAction( FaceControlSet &fcs, const QPoint&)
{
    std::unordered_set<FaceModelViewer*> vwrs;
    for ( FaceControl* fc : fcs)
    {
        // Is there a FaceControl on the target viewer of the same model? Remove if so since moving in fc to replace it.
        if ( _tviewer->get(fc->data()))
            _selector->removeFaceControl( _tviewer->get(fc->data()));

        _selector->setSelected( fc, false);  // De-select source FaceControl
        vwrs.insert(fc->viewer());  // Remember source viewer
        fc->viewer()->detach(fc);   // Detach from source viewer

        // If there are no other FaceContols in the target viewer, the visualisations for the moved view
        // are left as they are. Otherwise, the visualisations in the target viewer will be applied to the
        // moved in view - EXCEPTING the bounding view which is reapplied by the ending call to _selector->select().
        FaceControl* vfc = nullptr; // Will be the FaceControl to copy visualisation layers from if not null.
        if ( !_tviewer->attached().empty())
            vfc = _tviewer->attached().first();

        // vfc is now the FaceControl to copy visualisation layers from.

        _tviewer->attach( fc);  // Attach to target viewer
        if ( vfc != nullptr)    // Copy in the visualisation layers to the moved in view
        {
            fc->view()->remove();   // Remove existing visualisations
            for ( BaseVisualisation* vis : vfc->view()->visualisations())
                fc->view()->apply(vis);  // Does nothing if visualisation can't be applied
        }   // end if

        _selector->setSelected( fc, true);
    }   // end foreach

    // target viewer rendering will be updated, but source needs updating too so do that here.
    std::for_each( std::begin(vwrs), std::end(vwrs), [](auto v){ v->updateRender();});
    return true;
}   // end doAction
