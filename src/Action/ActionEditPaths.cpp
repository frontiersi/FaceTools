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

#include <ActionEditPaths.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::EventSet;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Interactor::MEEI;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FaceModel;


ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ico, MEEI* meei, QStatusBar* sbar)
    : ActionVisualise( _vis = new PathSetVisualisation( dn, ico)),
      _interactor( new PathSetInteractor( meei, _vis, sbar))
{
    // Leverage this action's reportFinished signal to propagate path edits.
    connect( _interactor, &ModelViewerInteractor::onChangedData, this, &ActionEditPaths::doOnEditedPath);
    setRespondToEventIfAllReady( PATHS_CHANGE, true);
}   // end ctor


ActionEditPaths::~ActionEditPaths()
{
    delete _interactor;
    delete _vis;
}   // end dtor


bool ActionEditPaths::doAction( FVS& fvs, const QPoint& mc)
{
    const FMS& fms = fvs.models();
    std::for_each( std::begin(fms), std::end(fms), [=](FM* fm){ _vis->refresh( fm);});
    return ActionVisualise::doAction(fvs, mc);
}   // end doAction


void ActionEditPaths::doAfterAction( EventSet& cs, const FVS& fvs, bool v)
{
    ActionVisualise::doAfterAction( cs, fvs, v);
    _interactor->setEnabled(isChecked());
}   // end doAfterAction


void ActionEditPaths::doOnEditedPath( const FV* fv)
{
    EventSet cset;
    cset.insert(PATHS_CHANGE);
    FVS fvs;
    fvs.insert(const_cast<FV*>(fv));
    emit reportFinished( cset, fvs, true);
}   // end doOnEditedPath
