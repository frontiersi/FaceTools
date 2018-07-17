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
#include <FaceModel.h>
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
using FaceTools::FaceModel;


ActionRadialSelect::ActionRadialSelect( const QString& dn, const QIcon& ico, FEEI* feei, QStatusBar* sbar)
    : ActionVisualise( _vis = new RadialSelectVisualisation( dn, ico)), _interactor(nullptr), _sbar(sbar)
{
    _interactor = new RadialSelectInteractor( feei, _vis);
    connect( _interactor, &RadialSelectInteractor::onSetNewCentre, this, &ActionRadialSelect::doOnSetNewCentre);
    connect( _interactor, &RadialSelectInteractor::onSetNewRadius, this, &ActionRadialSelect::doOnSetNewRadius);
}   // end ctor


ActionRadialSelect::~ActionRadialSelect()
{
    delete _interactor;
    delete _vis;
}   // end dtor


// private slot
void ActionRadialSelect::doOnSetNewCentre( FaceControl* fc, const cv::Vec3f& v)
{
    FaceModel* fm = fc->data();
    _vis->setCentre( fm, v);
    fm->updateRenderers();
}   // end doOnSetNewCentre


// private slot
void ActionRadialSelect::doOnSetNewRadius( FaceControl* fc, double r)
{
    FaceModel *fm = fc->data();
    _vis->setRadius( fm, r);
    fm->updateRenderers();
}   // end doOnSetNewRadius


void ActionRadialSelect::doAfterAction( ChangeEventSet& cs, const FaceControlSet& fcs, bool v)
{
    const static QString smsg( tr("Reposition with double-left-click and drag; change size with mouse-wheel."));
    if ( isChecked())
        _sbar->showMessage(smsg, 10000);    // 10 sec temp
    else
    {
        if ( _sbar->currentMessage() == smsg)
            _sbar->clearMessage();
    }   // end else
    ActionVisualise::doAfterAction( cs, fcs, v);
}   // end doAfterAction
