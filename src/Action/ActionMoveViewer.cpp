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

#include <ActionMoveViewer.h>
#include <ModelSelector.h>
#include <BaseVisualisation.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMoveViewer;
using FaceTools::Action::EventSet;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;


ActionMoveViewer::ActionMoveViewer( FMV *tv, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _sviewer(sv)
{
}   // end ctor


bool ActionMoveViewer::testReady( const FV* fv)
{
    return _sviewer && _sviewer->isAttached(fv);
}   // end testReady


// protected
bool ActionMoveViewer::doAction( FVS &fvs, const QPoint&)
{
    FMVS vwrs;
    for ( FV* fv : fvs)
    {
        FMV* sviewer = fv->viewer();        // Source viewer
        vwrs.insert( sviewer);              // Remember source viewer
        Action::ModelSelector::setSelected( fv, false); // De-select source FaceView

        const FM* fm = fv->data();
        // Does FV on target viewer have same model? Remove to replace if so.
        if ( _tviewer->get(fm))
            Action::ModelSelector::removeFaceView( _tviewer->get(fm));

        // If the target viewer has other FaceViews, they are applied to the FaceView being moved in (if possible)
        if ( !_tviewer->attached().empty())
        {
            const FV* tfv = _tviewer->attached().first();
            for ( BaseVisualisation* vl : tfv->visualisations())
            {
                if ( vl->isAvailable(fv->data()))
                    fv->apply(vl);
            }   // end for
        }   // end if

        fv->setViewer(_tviewer);    // Attach to target viewer (detaches from source)
        Action::ModelSelector::setSelected( fv, true);
    }   // end foreach

    // target viewer rendering will be updated, but source needs updating too so do that here.
    std::for_each( std::begin(vwrs), std::end(vwrs), [](FMV* v){ v->updateRender();});
    return true;
}   // end doAction
