/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionShowMeshInfo.h>
#include <FaceModel.h>
using FaceTools::Action::ActionShowMeshInfo;
using FaceTools::Action::ActionDiscardManifold;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Widget::MeshInfoDialog;
using FaceTools::FM;
using MS = FaceTools::Action::ModelSelector;


ActionShowMeshInfo::ActionShowMeshInfo( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _dialog(nullptr)
{
    addRefreshEvent( Event::MESH_CHANGE);
}   // end ctor


void ActionShowMeshInfo::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _dialog = new MeshInfoDialog(p);
}   // end postInit


bool ActionShowMeshInfo::update( Event e)
{
    const FM* fm = MS::selectedModel();
    if ( !fm || has( e, Event::CLOSED_MODEL))
        _dialog->hide();
    else
        _dialog->set( fm);
    return _dialog->isVisible();
}   // end update


bool ActionShowMeshInfo::isAllowed( Event) { return MS::isViewSelected();}


void ActionShowMeshInfo::doAction( Event)
{
    _dialog->show();
    _dialog->raise();
    _dialog->activateWindow();
}   // end doAction


Event ActionShowMeshInfo::doAfterAction( Event)
{
    MS::showStatus( "Showing Model Information", 5000);
    return Event::NONE;
}   // end doAfterAction
