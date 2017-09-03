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

#include <VisualisationToolBar.h>
#include <cassert>
using FaceTools::VisualisationToolBar;
using FaceTools::VisualisationInterface;


// public
VisualisationToolBar::VisualisationToolBar( const QActionGroup* viewGroup)
    : _viewGroup(viewGroup)
{
    setOrientation( Qt::Vertical);

    //int shortcutNum = 1;
    foreach ( const QAction* action, viewGroup->actions())
    {
        assert( action->parent() != NULL);
        //if ( shortcutNum < 10)  // Set a shortcut if we've not added too many visualisations
        //    action->setShortcut( QKeySequence( QString("%1").arg(shortcutNum++)));
        insertAction( NULL, action);
    }   // end foreach

    addSeparator();
    insertAction( NULL, ui->action_ShowBoundary);
    insertAction( NULL, ui->action_ShowLandmarks);
}   // end ctor

