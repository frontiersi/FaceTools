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

#ifndef FACE_TOOLS_FACE_ACTION_MANAGER_H
#define FACE_TOOLS_FACE_ACTION_MANAGER_H

/**
 * Manages interactions between loaded plugins and informs clients of their presence.
 * Plugins should be placed in appdir/plugins and are found after calling loadPlugins.
 * Upon the load of each plugin, the appropriate signal is fired and the dialog is
 * populated with the discovered plugins. Plugins are managed to communicate with one
 * another based on their signalling of milestones through their actions.
 */

#include "ActionSelect.h"
#include "FaceActionGroup.h"
#include <PluginsDialog.h>      // QTools

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT FaceActionManager : public QObject
{ Q_OBJECT
public:
    explicit FaceActionManager( QWidget* parent=NULL);
    ~FaceActionManager() override;

    void loadPlugins();                     // Call once after construction and connecting slots to signals
    QDialog* dialog() { return _pdialog;}   // Get a standard dialog which shows the loaded plugins.
    QAction* addAction( FaceAction*);       // Return added action's QAction if added okay (duplicates not allowed).

    // Return the selector for programmatic selection of FaceControl instances.
    ActionSelect* selector() { return &_selector;}

signals:
    void addedActionGroup( const FaceActionGroup&);
    void addedAction( const FaceAction&);

private slots:
    void addPlugin( QTools::PluginInterface*);
    void doOnActionStarting( const FaceControlSet*);
    void doOnActionFinished();
    void doOnReportChanges( const ChangeEventSet&, FaceControl*);
    void doOnSelect( FaceControl*, bool);
    void doOnRemove( FaceControl*);

private:
    ActionSelect _selector;
    QTools::PluginsDialog *_pdialog;
    std::unordered_set<FaceAction*> _actions;

    FaceActionManager( const FaceActionManager&);   // No copy
    void operator=( const FaceActionManager&);      // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
