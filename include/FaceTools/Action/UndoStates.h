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
    static void storeUndo( const FaceAction*, Event, bool autoRestore=true);

    // After storing an undo, scrap it (last one stored only) by specifying the associated model.
    // Can be helpful if needing to carry out a potential write action which doesn't end up
    // changing the model (so don't want to undo to a non-modified state).
    static void scrapLastUndo( const FM*);

    static bool canUndo();
    static bool canRedo();

    // Return the names of the actions that are top of
    // the undo and redo stacks for the currently selected model.
    static QString undoActionName();
    static QString redoActionName();

    static Event undo();
    static Event redo();

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
        std::deque<UndoState::Ptr> oldRedos;
    };  // end struct

    // Undo stacks per model and also a null entry!
    std::unordered_map<const FM*, Stacks> _stacks;

    QReadWriteLock _mutex;

    void _clear( const FM*);
    void _clear();
    void _storeUndo( const FaceAction*, Event, bool);
    void _scrapLastUndo( const FM*);
    bool _canUndo( const FM*);
    bool _canRedo( const FM*);
    QString _undoActionName();
    QString _redoActionName();
    Event _undo( const FM*);
    Event _redo( const FM*);
};  // end class

}}   // end namespaces

#endif
