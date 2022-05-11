/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Interactor/ContextMenuHandler.h>
#include <ModelSelect.h>
#include <FaceModelViewer.h>
#include <QApplication>
#include <QTime>
using FaceTools::Interactor::ContextMenuHandler;
using FaceTools::Action::FaceAction;
using FaceTools::Vis::FV;
using MS = FaceTools::ModelSelect;


ContextMenuHandler::Ptr ContextMenuHandler::create() { return Ptr( new ContextMenuHandler);}

// private
ContextMenuHandler::ContextMenuHandler() : _rDownTime(0)
{
    _cmenu.setToolTipsVisible(true);
}   // end ctor


void ContextMenuHandler::refresh()
{
    setEnabled( MS::selectedView());
}   // end refresh


void ContextMenuHandler::addAction( FaceAction* a)
{
    _cmenu.addAction( a->qaction());
    _actions.push_back(a);
}   // end addAction


void ContextMenuHandler::addSeparator()
{
    QAction* sep = new QAction;
    sep->setSeparator(true);
    _cmenu.addAction( sep);
}   // end addSeparator


bool ContextMenuHandler::doRightButtonDown()
{
    _rDownTime = 0;
    const vtkProp* prop = this->prop();
    const FV *fv = MS::selectedView();
    if (( fv && this->view() == fv) || (prop && const_cast<vtkProp*>(prop)->GetPickable()))
    {
        _rDownTime = QTime::currentTime().msecsSinceStartOfDay();
        _mDownPos = fv->viewer()->mouseCoords();
    }   // end if
    return false;
}   // end doRightButtonDown


bool ContextMenuHandler::doRightButtonUp()
{
    bool swallowed = false;
    const QPoint mpos = MS::selectedViewer()->mouseCoords();
    const int tnow = QTime::currentTime().msecsSinceStartOfDay();

    // Fifth of a second for acceptable right button down time
    if (( tnow - _rDownTime) < 200 && (mpos == _mDownPos))
    {
        swallowed = true;
        bool foundEnabledAction = false;
        for ( FaceAction* a : _actions)
            foundEnabledAction |= a->primeMousePos( mpos);    // Also refreshes state
        MS::updateRender();
        if ( foundEnabledAction)
            _cmenu.exec( QCursor::pos());
        for ( FaceAction* a : _actions) // Prime with null position
            a->primeMousePos();
    }   // end if

    _rDownTime = 0;
    _mDownPos = QPoint(-1,-1);
    return swallowed;
}   // end doRightButtonUp
