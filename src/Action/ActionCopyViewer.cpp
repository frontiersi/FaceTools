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

#include <ActionCopyViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionCopyViewer;
using FaceTools::Action::EventSet;
using FaceTools::Action::ModelSelector;
using FaceTools::Action::FaceAction;
using FaceTools::FaceModelViewer;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;


ActionCopyViewer::ActionCopyViewer( FaceModelViewer *tv, ModelSelector* s, FaceModelViewer *sv, const QString& dn, const QIcon& ico)
    : FaceAction(dn, ico), _tviewer(tv), _selector(s), _sviewer(sv)
{
}   // end ctor


bool ActionCopyViewer::testReady( const FV* fv)
{
    bool allowed = !_tviewer->isAttached(fv->data());   // Allowed if data not already on the target viewer
    if ( allowed && _sviewer != nullptr)
        allowed = _sviewer->isAttached(fv);
    return allowed;
}   // end testReady


// protected
bool ActionCopyViewer::doAction( FVS& fvs, const QPoint&)
{
    FVS fvsin = fvs;    // copy of pointers to source views
    fvs.clear();        // fvs will hold copied views on output
    for ( FV* fv : fvsin)
    {
        // If there exist FVs in the target viewer, copy their visualisations
        // to the new FV. Otherwise, copy over visualisations from source FV.
        FV* cfv = _tviewer->attached().first();
        if ( cfv == nullptr)
            cfv = fv;

        FM* fm = fv->data();
        fm->lockForRead();
        FV* nfv = _selector->addFaceView( fm, _tviewer); // Create the new FV from the underlying data.
        fm->unlock();

        // Copy over visualisations
        for ( auto vl : cfv->visualisations())
            nfv->apply(vl);

        _selector->setSelected( nfv, true); // Select the new FV
        fvs.insert(nfv);
    }   // end for
    return true;
}   // end doAction
