/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT FaceActionGroup : public FaceActionInterface
{ Q_OBJECT
public:
    FaceActionGroup();

    // Create menus / toolbars for the action group. By default, these functions create and return
    // new menus and toolbars containing the group's actions. The returned widgets should be parented
    // to an appropriate host widget. Derived types may override these functions to
    // return NULL if it isn't appropriate to create menus or toolbars.
    virtual QMenu* createMenu() const;
    virtual QToolBar* createToolBar() const;

    // If getWidget() is overridden to return something other than NULL, this function will return an
    // action with icon taken from getIcon() and name getDisplayName() that when triggered shows the
    // widget. If a keyboard shortcut is defined using getShortcut(), this is added to the action.
    // If widget() returns NULL (default), this function will also return NULL.
    QAction* qaction();

    // By default, these functions add all member actions in this group to the provided widgets.
    // Override in the derived type to add only subsets of the group's actions.
    virtual void addTo( QMenu*) const;
    virtual void addTo( QToolBar*) const;

    // QTools::PluginInterface
    QStringList getInterfaceIds() const override;
    FaceAction* getInterface( const QString&) const override;

protected slots:
    // Add new FaceAction instances. Returns true iff the action was successfully
    // added to the group. No actions with duplicate names allowed!
    bool addAction( FaceAction*);

private:
    QAction *_waction;
    std::unordered_map<std::string, FaceAction*> _actions;
    QList<FaceAction*> _alist;

    FaceActionGroup( const FaceActionGroup&);   // No copy
    void operator=( const FaceActionGroup&);    // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
