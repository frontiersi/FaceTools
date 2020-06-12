/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Interactor/ContextMenu.h>
#include <FaceModelViewer.h>
#include <QApplication>
#include <QTime>
using FaceTools::Interactor::ContextMenu;
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
    const QPoint mpos = MS::mousePos(); // Relative to viewer
    const vtkProp* prop = MS::mouseViewer()->getPointedAt( mpos);
    if ( prop && const_cast<vtkProp*>(prop)->GetPickable())
    {
        _rDownTime = QTime::currentTime().msecsSinceStartOfDay();
        _mDownPos = mpos;
    }   // end if
    return false;
}   // end rightButtonDown


bool ContextMenu::rightButtonUp()
{
    const QPoint mpos = MS::mousePos(); // Relative to viewer
    const int tnow = QTime::currentTime().msecsSinceStartOfDay();

    // Use double-click interval for acceptable right button down time
    if (( tnow - _rDownTime) < QApplication::doubleClickInterval() && (mpos == _mDownPos))
    {
        for ( FaceAction* a : _actions)
            a->primeMousePos( mpos);

        if ( _testEnabledActions() > 0)
        {
            MS::updateRender();
            _cmenu.exec( QCursor::pos());
        }   // end if

        // Need to deprime mouse position on all actions afterwards
        // since only a specific action *may* have been triggered.
        for ( FaceAction* a : _actions)
            a->primeMousePos();
    }   // end if

    _rDownTime = 0;
    _mDownPos = QPoint(-1,-1);
    return false;
}   // end rightButtonUp


size_t ContextMenu::_testEnabledActions() const
{
    size_t nenabled = 0;
    for ( FaceAction* a : _actions)
    {
        a->refreshState();  // Call on ALL actions since must recheck their enabled states at click
        if ( a->isEnabled())
            nenabled++;
    }   // end for
    return nenabled;
}   // end _testEnabledActions
