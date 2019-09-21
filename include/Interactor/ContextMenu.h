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

#ifndef FACE_TOOLS_CONTEXT_MENU_H
#define FACE_TOOLS_CONTEXT_MENU_H

#include "MouseHandler.h"
#include "../Action/FaceAction.h"
#include <QMenu>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT ContextMenu : public MouseHandler
{
public:
    ContextMenu();

    void addAction( Action::FaceAction*);
    void addSeparator();

private:
    bool rightButtonDown() override;
    bool rightButtonUp() override;

    int _rDownTime;
    QMenu _cmenu;
    std::vector<Action::FaceAction*> _actions;
    size_t _testEnabledActions() const;
};  // end class

}}   // end namespace

#endif
