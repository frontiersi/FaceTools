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

#include <ActionCopyViewer.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionCopyViewer;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Vis::BaseVisualisation;


ActionCopyViewer::ActionCopyViewer( FaceModelViewer *tv, ModelSelector* s, FaceModelViewer *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _selector(s), _sviewer(sv)
{
}   // end ctor


bool ActionCopyViewer::testReady( const FaceControl* fc)
{
    bool allowed = !_tviewer->isAttached(fc->data());   // Allowed if data not already on the target viewer
    if ( allowed && _sviewer != nullptr)
        allowed = _sviewer->isAttached(fc);
    return allowed;
}   // end testReady


// protected
bool ActionCopyViewer::doAction( FaceControlSet& fcs)
{
    for ( FaceControl* fc : fcs)
    {
        // If there exist FaceControl's in the target viewer, set their visualisations
        // to the copied over new FaceControl. Otherwise, copy over the visualisations
        // from the source FaceControl.
        FaceControl* cfc = _tviewer->attached().first();
        if ( cfc == nullptr)
            cfc = fc;

        // Create the new FaceControl from the underlying data.
        FaceControl* nfc = _selector->addFaceControl( fc->data(), _tviewer);

        // Copy over visualisations
        for ( BaseVisualisation* vis : fc->view()->visualisations())
            nfc->view()->apply(vis);    // Will do nothing if visualisation can't be applied
    }   // end for
    return true;
}   // end doAction


// protected
void ActionCopyViewer::doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool)
{ 
    cs.insert(VIEW_CHANGE);
}   // end doAfterAction
