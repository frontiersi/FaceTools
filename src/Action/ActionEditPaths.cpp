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

#include <ActionEditPaths.h>
#include <FaceControl.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <VtkTools.h>
using FaceTools::Action::ActionEditPaths;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ActionVisualise;
using FaceTools::Interactor::PathSetInteractor;
using FaceTools::Interactor::ModelViewerInteractor;
using FaceTools::Vis::PathSetVisualisation;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;


ActionEditPaths::ActionEditPaths( const QString& dn, const QIcon& ico, FEEI* feei, QStatusBar* sbar)
    : ActionVisualise( _vis = new PathSetVisualisation( dn, ico)), _interactor(nullptr), _sbar(sbar)
{
    _interactor = new PathSetInteractor( feei, _vis);
    // Hijack this action's reportFinished signal to propagate path edits.
    connect( _interactor, &ModelViewerInteractor::onChangedData, this, &ActionEditPaths::doOnEditedPath);
}   // end ctor


ActionEditPaths::~ActionEditPaths()
{
    delete _interactor;
    delete _vis;
}   // end dtor


void ActionEditPaths::doAfterAction( ChangeEventSet& cs, const FaceControlSet& fcs, bool v)
{
    if ( _sbar)
    {
        const static QString smsg( tr("Left-click and drag on path handles to move; right-click to add/remove/rename."));
        if ( isChecked())
            _sbar->showMessage(smsg, 10000);    // 10 sec temp
        else
        {
            if ( _sbar->currentMessage() == smsg)
                _sbar->clearMessage();
        }   // end else
    }   // end if
    ActionVisualise::doAfterAction( cs, fcs, v);
}   // end doAfterAction


void ActionEditPaths::doOnEditedPath( const FaceControl* fc)
{
    ChangeEventSet cset;
    cset.insert(METRICS_CHANGE);
    FaceControlSet fcs;
    fcs.insert(const_cast<FaceControl*>(fc));
    emit reportFinished( cset, fcs, true);
}   // end doOnEditedPath
