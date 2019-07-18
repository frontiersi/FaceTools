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

#ifndef FACE_TOOLS_ACTION_UNDO_STATES_H
#define FACE_TOOLS_ACTION_UNDO_STATES_H

#include "UndoState.h"
#include <QReadWriteLock>
#include <deque>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT UndoStates : public QObject
{ Q_OBJECT
public:
    // Undo/redo states per model cannot exceeed MAX_RESTORES.
    static const size_t MAX_RESTORES = 10;

    // Clear the undo/redo stacks for the given model (should happen on save/close).
    static void clear( const FM*);
    static void clear();    // Clear all undo/redos

    // Can call directly or call namespace function FaceTools::Action::storeUndo.
    static void storeUndo( const FaceAction*, EventGroup, bool autoRestore=true);

    static bool canUndo();
    static bool canRedo();

    // Return the names of the actions that are top of
    // the undo and redo stacks for the currently selected model.
    static QString undoActionName();
    static QString redoActionName();

    static void undo();
    static void redo();

    using Ptr = std::shared_ptr<UndoStates>;
    static Ptr get();

signals:
    void onUpdated();

private:
    static Ptr _singleton;

    struct Stacks
    {
        std::deque<UndoState::Ptr> undos;
        std::deque<UndoState::Ptr> redos;
    };  // end struct

    // Undo stacks per model and also a null entry!
    std::unordered_map<const FM*, Stacks> _stacks;

    QReadWriteLock _mutex;

    void _clear( const FM*);
    void _clear();
    void _storeUndo( const FaceAction*, EventGroup, bool);
    bool _canUndo();
    bool _canRedo();
    QString _undoActionName();
    QString _redoActionName();
    void _undo();
    void _redo();
};  // end class

}}   // end namespaces

#endif
