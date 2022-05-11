/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Action/ActionRenamePath.h>
#include <Interactor/PathsHandler.h>
#include <FaceModel.h>
#include <QSignalBlocker>
#include <cassert>
using FaceTools::Action::ActionRenamePath;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Interactor::PathsHandler;
using FaceTools::FVS;
using FaceTools::Vis::FV;
using FaceTools::FM;
using MS = FaceTools::ModelSelect;

#define _MAX_LABEL_CHARS 255


ActionRenamePath::ActionRenamePath( const QString& dn, const QIcon& ico, const QKeySequence &ks)
    : FaceAction(dn, ico, ks), _dialog(nullptr)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    connect( h, &PathsHandler::onEnterHandle, [this](){ this->refresh();});
    connect( h, &PathsHandler::onLeaveHandle, [this](){ this->refresh();});
    addRefreshEvent( Event::PATHS_CHANGE);
}   // end ctor


void ActionRenamePath::postInit()
{
    QWidget* prnt = static_cast<QWidget*>(parent());
    _dialog = new QInputDialog( prnt, Qt::Popup);
    _dialog->setLabelText( tr("New name:"));
    _dialog->setInputMode( QInputDialog::TextInput);
    // Prevent user from entering a new label that's too long.
    connect( _dialog, &QInputDialog::textValueChanged, this, &ActionRenamePath::_doOnTextValueChanged);
}   // end postInit


void ActionRenamePath::_doOnTextValueChanged( const QString &txt)
{
    if ( txt.size() > _MAX_LABEL_CHARS)
    {
        QSignalBlocker blocker(_dialog);
        _dialog->setTextValue( txt.left(_MAX_LABEL_CHARS));
    }   // end if
}   // end _doOnTextValueChanged


bool ActionRenamePath::isAllowed( Event)
{
    const PathsHandler *h = MS::handler<PathsHandler>();
    return MS::interactionMode() == IMode::CAMERA_INTERACTION && h->hoverPath();
}   // end isAllowed


bool ActionRenamePath::doBeforeAction( Event)
{
    const int pid = MS::handler<PathsHandler>()->hoverPath()->pathId();
    FM::RPtr fm = MS::selectedModelScopedRead();
    _label = fm->currentPaths().name(pid);

    _dialog->setTextValue( _label);
    _dialog->exec();

    bool go = false;
    const QString nlabel = _dialog->textValue();
    if ( nlabel != _label)
    {
        _label = nlabel;
        go = true;
    }   // end if

    return go;
}   // end doBeforeAction


void ActionRenamePath::doAction( Event)
{
    storeUndo(this, Event::PATHS_CHANGE);
    FM::WPtr fm = MS::selectedModelScopedWrite();
    const int pid = MS::handler<PathsHandler>()->hoverPath()->pathId();
    fm->renamePath( pid, _label);
}   // end doAction


Event ActionRenamePath::doAfterAction( Event) { return Event::PATHS_CHANGE;}
