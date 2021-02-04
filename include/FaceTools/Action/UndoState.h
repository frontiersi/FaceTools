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

#ifndef FACE_TOOLS_ACTION_UNDO_STATE_H
#define FACE_TOOLS_ACTION_UNDO_STATE_H

#include "FaceModelState.h"

namespace FaceTools { namespace Action {

class UndoStates;

/**
 * Called by an action at the end of FaceAction::doBeforeAction if desiring undo functionality.
 */
FaceTools_EXPORT void storeUndo( const FaceAction*, Event, bool autoRestore=true);
FaceTools_EXPORT void scrapLastUndo( const FM*);


class FaceTools_EXPORT UndoState
{
public:
    using Ptr = std::shared_ptr<UndoState>;

    // Get the model that was selected at creation time.
    inline FM* model() const { return _sfm;}

    // Set the data keyed by the given string.
    void setUserData( const QString&, const QVariant&);

    // Return the data keyed by the given string (no error checking!).
    QVariant userData( const QString&) const;

    inline const Event& events() const { return _egrp;}

    // Set the name of the undo state (defaults to the action's display name).
    void setName( const QString& nm) { _name = nm;}
    inline const QString& name() const { return _name;}

private:
    FaceAction* _action;
    Event _egrp;
    bool _autoRestore;  // Whether auto restoring state (true = _fstates) or manual (false = _udata).
    QString _name;
    FM *_sfm;
    std::vector<FaceModelState::Ptr> _fstates;  // The auto restore states (if being used)
    QMap<QString, QVariant> _udata; // The manually set state (if being used)

    UndoState( const FaceAction*, Event, bool);
    UndoState( const UndoState&) = delete;
    UndoState& operator=( const UndoState&) = delete;
    ~UndoState(){}

    inline bool isAutoRestore() const { return _autoRestore;}
    inline const FaceAction* action() const { return _action;}
    Event restore() const;   // Called by UndoStates
    static Ptr create( const FaceAction*, Event, bool autoRestore=false);  // Called by UndoStates

    friend class UndoStates;
};  // end class

}}   // end namespaces

#endif
