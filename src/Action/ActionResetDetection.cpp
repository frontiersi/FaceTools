/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <ActionResetDetection.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QMessageBox>
#include <cassert>
using FaceTools::Action::ActionResetDetection;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FMS;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionResetDetection::ActionResetDetection( const QString& dn, const QIcon& icon) : FaceAction(dn, icon) {}


bool ActionResetDetection::checkEnable( Event)
{
    const FV* fv = ModelSelector::selectedView();
    return fv && !fv->data()->landmarks().empty();
}   // end checkEnabled


bool ActionResetDetection::doBeforeAction( Event)
{
    QWidget* prnt = static_cast<QWidget*>(parent());
    static const QString msg = tr("This will also erase existing measurements; continue?");
    bool go = QMessageBox::Yes == QMessageBox::question( prnt, displayName(), msg,
                                  QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    return go;
}   // end doBeforeAction


void ActionResetDetection::doAction( Event)
{
    storeUndo(this, {Event::LANDMARKS_CHANGE, Event::ORIENTATION_CHANGE, Event::METRICS_CHANGE});

    FM* fm = ModelSelector::selectedModel();
    fm->lockForWrite();
    fm->setLandmarks( Landmark::LandmarkSet::create());
    fm->clearMetrics();
    fm->fixOrientation();
    fm->unlock();

    emit onEvent( {Event::LANDMARKS_CHANGE, Event::ORIENTATION_CHANGE, Event::METRICS_CHANGE});
}   // end doAction

