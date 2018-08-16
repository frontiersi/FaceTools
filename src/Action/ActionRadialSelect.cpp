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

#include <ActionRadialSelect.h>
#include <FaceControl.h>
#include <FaceTools.h>
#include <VtkTools.h>
#include <FaceModelViewer.h>
using FaceTools::Action::ActionRadialSelect;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ActionVisualise;
using FaceTools::Vis::RadialSelectVisualisation;
using FaceTools::Interactor::RadialSelectInteractor;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;


ActionRadialSelect::ActionRadialSelect( const QString& dn, const QIcon& ico, MEEI* meei, QStatusBar* sbar)
    : ActionVisualise( _vis = new RadialSelectVisualisation( dn, ico)),
      _interactor( new RadialSelectInteractor( meei, _vis, sbar))
{
    connect( meei, &MEEI::onEnterModel, [=](auto fc){ this->testSetEnabled( &meei->viewer()->mouseCoords());});
    connect( meei, &MEEI::onLeaveModel, [this](auto fc){ this->testSetEnabled( nullptr);});
}   // end ctor


ActionRadialSelect::~ActionRadialSelect()
{
    delete _interactor;
    delete _vis;
}   // end dtor


void ActionRadialSelect::doAfterAction( ChangeEventSet& cs, const FaceControlSet& fcs, bool v)
{
    ActionVisualise::doAfterAction( cs, fcs, v);
    _interactor->setEnabled( isChecked());
}   // end doAfterAction
