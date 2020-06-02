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

#ifndef FACE_TOOLS_ACTION_RENAME_PATH_H
#define FACE_TOOLS_ACTION_RENAME_PATH_H

#include "FaceAction.h"
#include <FaceTools/Interactor/PathsHandler.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionRenamePath : public FaceAction
{ Q_OBJECT
public:
    ActionRenamePath( const QString&, const QIcon&, Interactor::PathsHandler::Ptr);

    QString toolTip() const override { return "Name or rename the selected measurement.";}

protected:
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    Interactor::PathsHandler::Ptr _handler;
    Event _e;
};  // end class

}}   // end namespace

#endif
