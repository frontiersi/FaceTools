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

#ifndef FACE_TOOLS_ACTION_UNDO_STATE_H
#define FACE_TOOLS_ACTION_UNDO_STATE_H

#include "FaceModelState.h"

namespace FaceTools { namespace Action {

class UndoStates;

/**
 * Called by an action at the start of FaceAction::doAction if desiring undo functionality.
 * The first version of this function should be used for simple undo/redos based on events.
 * For more complex undo/redos, use the second signature of storeUndo which will require the
 * calling action to override FaceAction::makeUndoState and FaceAction::restoreState.
 */
FaceTools_EXPORT void storeUndo( const FaceAction*, EventGroup);
FaceTools_EXPORT void storeUndo( const FaceAction*);


class FaceTools_EXPORT UndoState
{
public:
    using Ptr = std::shared_ptr<UndoState>;
    static Ptr create( const FaceAction*, EventGroup, bool autoRestore=false);

    // Get the model that was selected at creation time.
    inline FM* model() const { return _sfm;}

    // Return the action this UndoState was created for/by.
    inline const FaceAction* action() const { return _action;}

    inline const EventGroup& events() const { return _egrp;}

    // Set the name of the undo state (defaults to the action's display name).
    void setName( const QString& nm) { _name = nm;}
    inline const QString& name() const { return _name;}

    inline bool isAutoRestore() const { return _autoRestore;}

    // Set the data keyed by the given string.
    void setUserData( const QString&, const QVariant&);

    // Return the data keyed by the given string (no error checking!).
    QVariant userData( const QString&) const;

    void restore() const;   // Called by UndoStates

private:
    FaceAction* _action;
    EventGroup _egrp;
    bool _autoRestore;  // Whether auto restoring state (true = _fstates) or manual (false = _udata).
    QString _name;
    FM *_sfm;
    std::vector<FaceModelState::Ptr> _fstates;  // The auto restore states (if being used)
    QMap<QString, QVariant> _udata; // The manually set state (if being used)

    UndoState( const FaceAction*, EventGroup, bool);
    UndoState( const UndoState&) = delete;
    UndoState& operator=( const UndoState&) = delete;
    ~UndoState(){}
};  // end class

}}   // end namespaces

#endif
