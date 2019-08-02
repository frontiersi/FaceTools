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

#ifndef FACE_TOOLS_FACE_ACTION_GROUP_H
#define FACE_TOOLS_FACE_ACTION_GROUP_H

#include "FaceAction.h"
#include <QMenu>
#include <QList>
#include <QToolBar>
#include <unordered_map>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceActionGroup : public FaceActionInterface
{ Q_OBJECT
public:
    FaceActionGroup();

    // QTools::PluginInterface
    QStringList interfaceIds() const override;
    FaceAction* iface( const QString&) const override;

protected slots:
    // Add new FaceAction instances. Returns true iff the action was successfully
    // added to the group. No actions with duplicate names allowed!
    bool addAction( FaceAction*);

private:
    QAction *_waction;
    std::unordered_map<std::string, FaceAction*> _actions;
    QList<FaceAction*> _alist;

    FaceActionGroup( const FaceActionGroup&) = delete;
    void operator=( const FaceActionGroup&) = delete;
};  // end class

}}   // end namespaces

#endif
