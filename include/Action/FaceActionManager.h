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
 *
 * FaceAction instances that are registered with the FaceActionManager call the following
 * functions immediately after entering the process function (either directly or by
 * triggering the exposed QAction) (signals are shown here with the EMIT keyword):
 *
 * EMIT reportStarting()
 * started = doBeforeAction()
 * IF started THEN
 *    v = doAction( readySet)
 *    doAfterAction( readySet, v)
 *    FOR FaceControls f IN readySet
 *       EMIT reportChanged(f)
 *    ENDFOR
 * ENDIF
 * EMIT reportFinished()
 */

#include "FaceAction.h"
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
    QAction* addAction( FaceAction*);       // Returns the added action's internal QAction if added okay (duplicate instances not allowed).

    void printActionInfo( std::ostream& os) const;

public slots:
    void setSelected( FaceControl*, bool);  // Forwards through to all actions - always call in GUI thread!
    void remove( FaceControl*);             // Cause all FaceActions to discard any data concerning the FaceControl.

signals:
    // Signal that the given action is about to start or has just finished
    // operating over the given set of FaceControl instances. The reportFinished
    // signal is only ever emitted if the parameter action was actually performed
    // (no signals are emitted for cancelled actions).
    void reportStarting( const FaceAction&, const FaceControlSet&);
    void reportFinished( const FaceAction&, const FaceControlSet&);

    void addedActionGroup( const FaceActionGroup&);
    void addedAction( const FaceAction&);

private slots:
    void addPlugin( QTools::PluginInterface*);
    void doOnActionStarting( const FaceControlSet*);
    void doOnActionFinished( const FaceControlSet*);

private:
    QTools::PluginsDialog *_pdialog;
    std::unordered_set<FaceAction*> _actions;
    void connectActionPair( FaceAction*, FaceAction*);
    void printActionComms( std::ostream&, const FaceAction*) const;
    FaceActionManager( const FaceActionManager&);   // No copy
    void operator=( const FaceActionManager&);      // No copy
};  // end class

}   // end namespace
}   // end namespace

#endif
