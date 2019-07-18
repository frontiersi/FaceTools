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

#include <FaceModel.h>

namespace FaceTools { namespace Action {

class UndoStates;

/**
 * Called by an action at the start of FaceAction::doAction if desiring undo functionality.
 */
FaceTools_EXPORT void storeUndo( const FaceAction*, EventGroup, bool autoRestore=true);


class FaceTools_EXPORT UndoState
{
public:
    using Ptr = std::shared_ptr<UndoState>;
    static Ptr create( EventGroup, bool autoRestore=false);

protected:
    FM *_fm;
    FM _backm;
    cv::Matx44d _tmat;  // Affine transforms
    cv::Matx44d _omat;  // Orientation transforms

private:
    FaceAction* _action;
    EventGroup _egrp;
    bool _autoRestore;
    void restore() const;
    friend class UndoStates;
};  // end struct

}}   // end namespaces

#endif
