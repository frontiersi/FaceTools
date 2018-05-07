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
#include <ModelViewer.h>
using FaceTools::Interactor::ContextMenuInteractor;


// public
ContextMenuInteractor::ContextMenuInteractor( QMenu* cm)
{
    setContextMenu(cm);
}   // end ctor


void ContextMenuInteractor::setContextMenu( QMenu* cm) { _cmenu = cm;}


bool ContextMenuInteractor::rightButtonDown( const QPoint& p)
{
    bool swallowed = false;
    if ( _cmenu)
    {
        const vtkProp* prop = viewer()->getPointedAt(p);
        if ( prop && const_cast<vtkProp*>(prop)->GetPickable())
        {
            swallowed = true;
            _cmenu->exec( viewer()->mapToGlobal(p));
        }   // end if
    }   // end if
    return swallowed;
}   // end rightButtonDown
