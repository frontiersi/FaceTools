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

#include <ContextMenuInteractor.h>
#include <FaceAction.h>
#include <ModelViewer.h>
#include <QDateTime>
#include <QApplication>
using FaceTools::Interactor::ContextMenuInteractor;
using FaceTools::Action::FaceAction;


// public
ContextMenuInteractor::ContextMenuInteractor()
    : _rDownTime(0)
{
}   // end ctor


void ContextMenuInteractor::addAction( FaceAction* a)
{
    _cmenu.addAction( a->qaction());
    _actions.push_back(a);
}   // end addAction


void ContextMenuInteractor::addSeparator()
{
    QAction* sep = new QAction;
    sep->setSeparator(true);
    _cmenu.addAction( sep);
}   // end addSeparator


bool ContextMenuInteractor::rightButtonDown( const QPoint& p)
{
    _rDownTime = 0;
    const vtkProp* prop = viewer()->getPointedAt(p);
    if ( prop && const_cast<vtkProp*>(prop)->GetPickable())
        _rDownTime = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    return false;
}   // end rightButtonDown


bool ContextMenuInteractor::rightButtonUp( const QPoint& p)
{
    const qint64 tnow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    // Use double-click interval for acceptable right button down time
    if (( tnow - _rDownTime) < (QApplication::doubleClickInterval()/2))
    {
        if ( testEnabledActions() > 0)
            _cmenu.exec( viewer()->mapToGlobal(p));
    }   // end if
    _rDownTime = 0;
    return false;
}   // end rightButtonUp


size_t ContextMenuInteractor::testEnabledActions() const
{
    size_t nenabled = 0;
    for ( FaceAction* a : _actions)
    {
        if ( a->testSetEnabled())
            nenabled++;
    }   // end for
    return nenabled;
}   // end testEnabledActions
