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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_H
#define FACE_TOOLS_ACTION_FACE_ACTION_H

#include "UndoStates.h"
#include "ModelSelector.h"
#include "FaceActionWorker.h"
#include <QTools/PluginInterface.h>
#include <QAction>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT FaceAction : public QTools::PluginInterface
{ Q_OBJECT
public:
    FaceAction();
    FaceAction( const QString& displayName);
    FaceAction( const QString& displayName, const QIcon& icon);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence&);

    const std::string& debugName() const;

    /**
     * Dynamically change the name of this action.
     */
    void setDisplayName( const QString&);

    /**
     * Dynamically change this action's tooltip.
     */
    void setToolTip( const QString&);
    QString toolTip() const override { return _action.toolTip();}

    QString displayName() const override;
    const QIcon* icon() const override;
    const QKeySequence* shortcut() const override;

    /**
     * Returns the internal QAction.
     * Triggering the action calls this action's execute function.
     */
    QAction* qaction() { return &_action;}

    /**
     * Returns which menu to attach this action to (none by default).
     * If the specified menu cannot be found, the action is left unconnected.
     */
    virtual QString attachToMenu() { return "";}

    /**
     * Returns which tool bar to attach this action to (none by default).
     * If the specified tool bar cannot be found, the action is left unconnected.
     */
    virtual QString attachToToolBar() { return "";}

    /**
     * Return the location of a single file to be added to help (usually HTML).
     * The provided token should be used as the parameter when emitting onShowHelp
     * to show the help file returned by this function.
     */
    virtual QString helpFile( const QString& helpToken) { return "";}

    void setCheckable( bool, bool initialCheckState);
    void setChecked( bool);
    bool isChecked() const { return _action.isChecked();}

    bool isEnabled() const { return _action.isEnabled();}

    /**
     * Only unlocked actions (unlocked is the default) can be enabled and executed.
     * It doesn't matter if a descendent class's isAllowed() function returns
     * true since the lock value is checked here in the parent class before enabling.
     * Calling setLocked will call refresh on this action immediately afterwards.
     */
    bool isUnlocked() const { return _unlocked;}
    void setLocked( bool);

    /**
     * Returns true if this action is currently within the doAction() function
     * (which may be called from a separate thread). FaceActionManager uses this
     * to determine which actions can be set ready. Running actions refresh themselves
     * immediately after doAfterAction() returns and before the final onEvent is emitted.
     */
    bool isWorking() const { return _isWorking;}

    /**
     * Some actions may need the mouse position at time of actioning. This may be different from the
     * mouse position during the call to doBeforeAction since the action can be executed by a variety
     * of means. This function primes the action with a default mouse position before the action is
     * executed. This primed mouse position is reset after the action finishes so must be reprimed by
     * the client if necessary. After priming the mouse position, refresh is called to allow the
     * action to test its readiness based on the provided mouse position. If the action is not
     * enabled with the primed mouse position, the mouse position is deprimed (set as -1,-1).
     * Returns true iff the action is allowed to execute after priming.
     */
    bool primeMousePos( const QPoint& p=QPoint(-1,-1));

    /**
     * Derived types can specify the events that they wish to be purged for, triggered, and/or refreshed by.
     * For any event, purging comes before refreshing (checking state/enable) which comes before triggering.
    */
    void addPurgeEvent( Event);
    bool isPurgeEvent( Event) const;
    Event purgeEvents() const { return _pevents;}

    void addTriggerEvent( Event);
    bool isTriggerEvent( Event) const;
    Event triggerEvents() const { return _tevents;}

    void addRefreshEvent( Event);
    bool isRefreshEvent( Event) const;
    Event refreshEvents() const { return _revents;}

signals:
    void onEvent( Event);   // Report to others that state changing event(s) have occurred.
    void onShowHelp( const QString& helpToken);   // Show the given help by passing token (see helpFile).

public slots:
    /**
     * Function execute() is the main entry point for clients to carry out this action. It can be called
     * explicitly whether or not this FaceAction is enabled, but is usually called via triggering the
     * internal enabled QAction returned from qaction(). When triggered by the action, the checkAction
     * parameter takes on the value of QAction::isChecked. When calling externally, the check state of the
     * QAction will be set according to the parameter (true by default).
     * The following sequence of calls are made:
     * 1) isAllowed (in GUI thread)
     * 2) doBeforeAction (in GUI thread)
     * 3) doAction  (in GUI thread OR background thread)
     * 4) doAfterAction (in GUI thread)
     * 5) refresh (in GUI thread)
     * 6) onEvent( Event::NONE)
     * Calls after 2 are only made if doBeforeAction returns true. If doBeforeAction returns false,
     * then refresh is called followed by the emission of signal onEvent( Event::ACT_CANCELLED).
     * Note that BEFORE doBeforeAction until AFTER doAfterAction (or after false is returned from
     * doBeforeAction), the action will return false from isEnabled() and true from _isRunning().
     * True is eventually returned from execute() iff doBeforeAction returns true.
     * Set the event to something other than Event::NONE if triggered.
     */
    bool execute( Event e=Event::NONE);

    /**
     * First calls update on self and sets the action's checked state to the returned value.
     * Then calls isAllowed on self and sets the action's enabled state to the returned value.
     * This function is called at the end of this action being executed just prior to the
     * final onEvent signal being emitted. It can also be called internally at any point.
     */
    void refresh( Event e=Event::NONE);

    /**
     * For asynchronous actions, this function is called if the action times out.
     * The action should stop what it's doing and finish immediately.
     */
    virtual void endNow();

protected:
    /**
     * Called on self at the end of _init(). Override to manually adjust details of action/icon assignment here.
     */
    virtual void postInit() {}

    /**
     * Set asynchronous execution or not on the next call to execute().
     * Default is synchronous (blocking calls) in the GUI thread.
     */
    void setAsync( bool async);
    bool isAsync() const { return _doasync;}

    /**
     * Returns the client set mouse position. Returns (-1,-1) if not set.
     * Always reset to (-1,-1) after doAfterAction executes.
     */
    const QPoint& primedMousePos() const { return _mpos;}

    /**
     * Derived actions may cache data against a model. This function is called to purge these data
     * because of events specified using addPurgeEvent or due to other conditions that can invalidate
     * data such as closing the model.
     */
    virtual void purge( const FM*){}

    /**
     * Derived actions should test the application's state, configure themselves accordingly,
     * and return whether they should be checked or not (the returned value is used as the parameter
     * to setChecked). The returned value is ignored if this action is not checkable. The passed
     * in Event is the reason for calling this function.
     */
    virtual bool update( Event) { return isChecked();}

    /**
     * Called immediately after update, this function should return whether or not this action
     * should be enabled. Only enabled actions are available to the user and to be executed
     * in response to received events.
     */
    virtual bool isAllowed( Event) { return _unlocked;}

    /**
     * doBeforeAction always occurs in the GUI thread so this is where to show dialogs etc in order to get
     * user input. It is executed immediately before doAction which is only entered if the return value
     * from doBeforeAction is true. Note that doAction may run in a different thread than the GUI thread.
     */
    virtual bool doBeforeAction( Event){ return true;}

    /**
     * Implement the action; execute() decides whether it runs asynchronously or not (or at all).
     * If doAction runs asynchronously, defer all GUI updates etc to doAfterAction(). In particular,
     * do NOT emit doEvent signals from a non-GUI thread - defer until within doAfterAction.
     */
    virtual void doAction( Event){}

    /**
     * Called within the GUI thread immediately on the completion of doAction. This is where GUI
     * elements (dialogs etc) shown in doBeforeAction should be hidden or rendering updates made.
     * For asynchronous actions, emit doEvent if necessary from within this function.
     * By default, this function just displays the name of this action in the status bar for two seconds
     * if the action is not asynchronous, and for five seconds and prefixed with "Finished " if the
     * action is asynchronous. Return the event(s) to emit as a result of performing the action.
     * If this function is not overridden, the always emitted event Event::NONE is emitted.
     */
    virtual Event doAfterAction( Event);

    /**
     * Override this function to provide custom recording of state for undo/redo functionality.
     * Necessary to implement this if UndoStates::storeUndo is called with autoRestore=false.
     * The set UndoState object should contain sufficient data to perform restoreState with.
     * To enable undo/redo functionality, the action should call UndoStates::storeUndo( this)
     * before carrying out modifications that should be allowed to be undone.
     */
    virtual void saveState( UndoState&) const;   // Has default ERROR implementation!

    /**
     * Override this function to provide custom restoring of state for undo/redo functionality.
     * Necessary to implement this if UndoStates::storeUndo is called with autoRestore=false.
     * The action should restore state from an UndoState object. The passed in UndoState object
     * will have been previously set by this action's own overridden saveState function
     * and so this action should use knowledge of the makeup of that object to restore state.
     * During this call, the model that was selected when saveState was called will be locked
     * in write mode. After returning, the events originally passed to UndoStates::storeUndo are
     * emitted and the memory used by the UndoState object is reclaimed.
    */
    virtual void restoreState( const UndoState&);   // Has default ERROR implementation!

private slots:
    void _endExecute( Event);

private:
    QAction _action;
    QString _dname;
    std::string _debugName;
    const QIcon _icon;
    const QKeySequence _keys;
    bool _doasync;
    bool _isWorking;
    FaceActionWorker *_worker;
    bool _unlocked; // If true, this action is enabled (true by default)
    Event _pevents; // Purge events
    Event _tevents; // Trigger events
    Event _revents; // Refresh events
    QPoint _mpos;   // The primed mouse position

    void _pinit();

    /**
     * FaceActionManager calls _init() on a FaceAction when being added to it. This sets up the internal QAction
     * instance and ensures that triggering the QAction causes the execute() function to run on this FaceAction.
     * It also adds the text to the action as getDisplayName(). If unhappy with any of this, make adjustments in
     * postInit() which is called at the end of _init().
     */
    void _init( QWidget* parent=nullptr);

    friend class FaceActionWorker;
    friend class FaceActionManager;
    friend class UndoStates;
    friend class UndoState;
    FaceAction( const FaceAction&) = delete;
    void operator=( const FaceAction&) = delete;
};  // end class

}}   // end namespaces

#endif
