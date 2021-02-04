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

#ifndef FACE_TOOLS_FACE_ACTION_MANAGER_H
#define FACE_TOOLS_FACE_ACTION_MANAGER_H

#include "FaceAction.h"

/**
 * IMPORTANT:
 * Before creating the singleton FaceActionManager, ensure that all FaceModelViewer instances
 * have been added using ModelSelect::addViewer since all interactors and mouse handlers
 * will attach themselves to all available viewers on creation.
 */

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceActionManager : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<FaceActionManager>;

    // Return added action's QAction if added okay (duplicates not allowed).
    // Call finalise after registering all.
    static QAction* registerAction( FaceAction*, QWidget *parent);

    // Call after all actions have been registered.
    static void finalise();

    // Raise the given event (always raised in the GUI thread).
    static void raise( Event);

    static Vis::FV* close( const FM*);

    static FaceActionManager* get();    // For connecting to signals

signals:
    void onUpdateSelected();
    void onShowHelp( const QString&);
    void _selfRaise( Event);

private slots:
    void _doRaise( Event e=Event::NONE);

private:
    static FaceActionManager::Ptr s_singleton;
    std::vector<FaceAction*> _actions; // Actions in registered order

    int _lvl;
    std::unordered_map<FaceAction*, Event> _acted;

    FaceActionManager();
    FaceActionManager( const FaceActionManager&) = delete;
    void operator=( const FaceActionManager&) = delete;
};  // end class

}}   // end namespaces

#endif
