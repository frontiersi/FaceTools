/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_VISUALISATIONS_MANAGER_H
#define FACE_TOOLS_VISUALISATIONS_MANAGER_H

#include "ActionVisualise.h"
#include <QActionGroup>

namespace FaceTools {
namespace Action {
class FaceActionManager;

class FaceTools_EXPORT VisualisationsManager : public QObject
{ Q_OBJECT
public:
    VisualisationsManager();

    void init( FaceActionManager*);
    void add( FaceAction*); // Does nothing if parameter cannot be cast to ActionVisualise.

    // Return a concatenation of the exclusive and non-exclusive actions with a dividing separator.
    // If there are no non-exclusive actions, no separator is added and only a list of the
    // exclusive actions is returned (which may be empty).
    QList<QAction*> actions() const;

private:
    QActionGroup _evis;     // Exclusive visualisation actions
    QActionGroup _nvis;     // Non-exclusive visualisation actions
    ActionVisualise* _vact;
    std::unordered_set<FaceAction*> _actions;

    VisualisationsManager( const VisualisationsManager&) = delete;
    void operator=( const VisualisationsManager&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
