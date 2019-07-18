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

#include <ContextMenu.h>
#include <FaceAction.h>
#include <ModelViewer.h>
#include <QTime>
#include <QApplication>
using FaceTools::Interactor::ContextMenu;
using FaceTools::Interactor::MVI;
using FaceTools::Action::FaceAction;
using MS = FaceTools::Action::ModelSelector;


ContextMenu::ContextMenu() : _rDownTime(0)
{
    _cmenu.setToolTipsVisible(true);
}   // end ctor


void ContextMenu::addAction( FaceAction* a)
{
    _cmenu.addAction( a->qaction());
    _actions.push_back(a);
}   // end addAction


void ContextMenu::addSeparator()
{
    QAction* sep = new QAction;
    sep->setSeparator(true);
    _cmenu.addAction( sep);
}   // end addSeparator


bool ContextMenu::rightButtonDown()
{
    _rDownTime = 0;
    const vtkProp* prop = MS::mouseViewer()->getPointedAt( MS::mousePos());
    MS::mouseViewer()->updateRender();
    if ( prop && const_cast<vtkProp*>(prop)->GetPickable())
        _rDownTime = QTime::currentTime().msecsSinceStartOfDay();
    return false;
}   // end rightButtonDown


bool ContextMenu::rightButtonUp()
{
    const int tnow = QTime::currentTime().msecsSinceStartOfDay();
    // Use double-click interval for acceptable right button down time
    if (( tnow - _rDownTime) < QApplication::doubleClickInterval())
    {
        const QPoint mpos = MS::mousePos(); // Relative to viewer
        for ( FaceAction* a : _actions)
            a->primeMousePos( mpos);

        if ( testEnabledActions() > 0)
            _cmenu.exec( QCursor::pos());

        // Need to deprime mouse position on all actions afterwards
        // since only a specific action *may* have been triggered.
        for ( FaceAction* a : _actions)
            a->primeMousePos();
    }   // end if
    _rDownTime = 0;
    return false;
}   // end rightButtonUp


size_t ContextMenu::testEnabledActions() const
{
    size_t nenabled = 0;
    for ( FaceAction* a : _actions)
    {
        // Need to call on ALL actions since we need to recheck their enabled states
        a->refreshState();
        if ( a->isEnabled())
            nenabled++;
    }   // end for
    MS::mouseViewer()->updateRender();
    return nenabled;
}   // end testEnabledActions
