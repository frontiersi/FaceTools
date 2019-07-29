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

#include <ActionShowModelProperties.h>
#include <FaceModel.h>
using FaceTools::Action::ActionShowModelProperties;
using FaceTools::Action::ActionDiscardManifold;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::ModelPropertiesDialog;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionShowModelProperties::ActionShowModelProperties( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _dialog(nullptr)
{
}   // end ctor


void ActionShowModelProperties::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _dialog = new ModelPropertiesDialog(p);
}   // end postInit


ActionShowModelProperties::~ActionShowModelProperties()
{
    delete _dialog;
}   // end dtor


bool ActionShowModelProperties::checkState( Event e)
{
    if ( EventGroup(e).has(Event::CLOSED_MODEL) && !MS::selectedModel())
        _dialog->hide();
    return !_dialog->isHidden();
}   // end checkState


bool ActionShowModelProperties::checkEnable( Event)
{
    const FM* fm = MS::selectedModel();
    _dialog->set( fm);
    return fm != nullptr;
}   // end checkEnable


void ActionShowModelProperties::doAction( Event)
{
    _dialog->show();
    _dialog->raise();
    _dialog->activateWindow();
}   // end doAction


void ActionShowModelProperties::doAfterAction( Event)
{
    MS::showStatus( "Showing Model Properties", 5000);
}   // end doAfterAction
