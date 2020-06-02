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

#ifndef FACE_TOOLS_FACE_ACTION_MANAGER_H
#define FACE_TOOLS_FACE_ACTION_MANAGER_H

#include "FaceAction.h"
#include <QMutex>

/**
 * IMPORTANT:
 * Before creating the singleton FaceActionManager, ensure that all FaceModelViewer instances
 * have been added using ModelSelector::addViewer since all interactors and mouse handlers
 * will attach themselves to all available viewers on creation.
 */

namespace FaceTools { namespace Action {

class ModelSelector;

class FaceTools_EXPORT FaceActionManager : public QObject
{ Q_OBJECT
public:
    using Ptr = std::shared_ptr<FaceActionManager>;

    // Get (creating if necessary) the static singleton.
    // Providing the parent widget is only necessary for the first (creating) call.
    // For calls where the singleton is already present, the parent parameter is ignored.
    static Ptr get( QWidget* parent=nullptr);

    static QAction* registerAction( FaceAction*);       // Return added action's QAction if added okay (duplicates not allowed).

    static void close( const FM*);

public slots:
    void doEvent( const Event &e=Event::NONE);

signals:
    void onRefresh();
    void onUpdate( const FM*);
    void onShowHelp( const QString&);
    void onRegisteredAction( FaceAction*);

private:
    static FaceActionManager::Ptr s_singleton;
    static QMutex s_closeLock;

    QWidget *_parent;
    std::unordered_set<FaceAction*> _actions;

    FaceActionManager();
    FaceActionManager( const FaceActionManager&) = delete;
    void operator=( const FaceActionManager&) = delete;
};  // end class

}}   // end namespaces

#endif
