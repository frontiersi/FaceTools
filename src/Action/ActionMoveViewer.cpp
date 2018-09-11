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
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionMoveViewer;
using FaceTools::Action::EventSet;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using FaceTools::FMVS;
using FaceTools::FMV;
using FaceTools::FVS;
using FaceTools::FM;


ActionMoveViewer::ActionMoveViewer( FMV *tv, ModelSelector* s, FMV *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _selector(s), _sviewer(sv)
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
        _selector->setSelected( fv, false); // De-select source FaceView

        const FM* fm = fv->data();
        // Does FV on target viewer have same model? Remove to replace if so.
        if ( _tviewer->get(fm))
        {
            assert(_tviewer->get(fm) != fv);
            _selector->removeFaceView( _tviewer->get(fm));
        }   // end if

        // If the target viewer has other FaceViews, they are applied to the FaceView being moved in.
        if ( !_tviewer->attached().empty())
        {
            const FV* tfv = _tviewer->attached().first();
            for ( auto vl : tfv->visualisations())
                fv->apply(vl);
        }   // end if

        fv->setViewer(_tviewer);    // Attach to target viewer (detaches from source)
        _selector->setSelected( fv, true);
    }   // end foreach

    // target viewer rendering will be updated, but source needs updating too so do that here.
    std::for_each( std::begin(vwrs), std::end(vwrs), [](auto v){ v->updateRender();});
    return true;
}   // end doAction
