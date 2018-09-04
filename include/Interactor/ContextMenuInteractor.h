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

#ifndef FACE_TOOLS_CONTEXT_MENU_INTERACTOR_H
#define FACE_TOOLS_CONTEXT_MENU_INTERACTOR_H

#include "ModelViewerInteractor.h"
#include <FaceAction.h>
#include <QMenu>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ContextMenuInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    ContextMenuInteractor();

    void addAction( Action::FaceAction*);
    void addSeparator();

private:
    qint64 _rDownTime;
    QMenu _cmenu;
    std::vector<Action::FaceAction*> _actions;

    bool rightButtonDown( const QPoint&) override;
    bool rightButtonUp( const QPoint&) override;
    size_t testEnabledActions( const QPoint&) const;
};  // end class

}   // end namespace
}   // end namespace

#endif
