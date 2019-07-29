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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_H
#define FACE_TOOLS_ACTION_FACE_ACTION_H

#include "UndoStates.h"
#include <PluginInterface.h>    // QTools
#include <ModelSelector.h>
#include <QAction>

namespace FaceTools { namespace Action {

// Must remain pure virtual so Qt can make it a plugin interface.
class FaceTools_EXPORT FaceActionInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceActionInterface(){}
};  // end class


class FaceTools_EXPORT FaceAction : public FaceActionInterface
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

    QString displayName() const override;
    const QIcon* icon() const override;
    const QKeySequence* shortcut() const override;

    /**
     * Returns the internal QAction.
     * Triggering the action calls this action's execute function.
     */
    QAction* qaction() { return &_action;}

    /**
     * Actions can use this flag to know if they should ask the user for confirmation
     * before carrying out an action. This should be tested within overridden doBeforeAction.
     * This setting is true by default.
     */
    void setRequireConfirm( bool v) { _reqConfirm = v;}
    bool requireConfirm() const { return _reqConfirm;}

    void setCheckable( bool b, bool initialCheckState);
    void setChecked( bool b) { _action.setChecked(b);}
    bool isChecked() const { return _action.isChecked();}

    bool isEnabled() const { return _action.isEnabled();}

    /**
     * Only unlocked actions (unlocked is the default) can be enabled and executed.
     * It doesn't matter if a descendent class's checkEnable() function returns
     * true since the lock value is checked here in the parent class before enabling.
     * Calling setLocked will call refreshState on this action immediately afterwards.
     */
    bool isUnlocked() const { return _unlocked;}
    void setLocked( bool);

    /**
     * Some actions may need the mouse position at the time they are actioned. This may be
     * different from the current mouse position during the call to doBeforeAction since
     * the action can be executed by a variety of means. Calling this function allows clients
     * to prime this action with a default mouse position before the action is executed.
     * This primed mouse position is reset after the action finishes executing so must be
     * reprimed by the client if necessary.
     */
    void primeMousePos( const QPoint& p=QPoint(-1,-1));

    /**
     * Derived types can specify the events that they wish to be purged for, and/or triggered by.
     * For any event, purging comes before refreshing (checking state/enable) which comes before triggering.
    */
    void addPurgeEvent( EventGroup);
    bool isPurgeEvent( EventGroup) const;
    void addTriggerEvent( EventGroup);
    bool isTriggerEvent( EventGroup) const;

    /**
     * Function execute() is the main entry point for clients to carry out this action. It can be called
     * explicitly whether or not this FaceAction is enabled, but is usually called via triggering the
     * internal enabled QAction returned from qaction(). When triggered by the action, the checkAction
     * parameter takes on the value of QAction::isChecked. When calling externally, the check state of the
     * QAction will be set according to the parameter (true by default).
     * The following sequence of calls are made:
     * 1) doBeforeAction (in GUI thread)
     * 2) doAction  (in GUI thread OR background thread)
     * 3) doAfterAction (in GUI thread)
     * 4) refreshState (in GUI thread)
     * 5) onEvent( Event::ACT_COMPLETE)
     * Calls after 1 are only made if doBeforeAction returns true. If doBeforeAction returns false,
     * then refreshState is called followed by the emission of signal onEvent( Event::ACT_CANCELLED).
     * Note that BEFORE doBeforeAction until AFTER doAfterAction (or after false is returned from
     * doBeforeAction), the action will return false from isEnabled() and true from isRunning().
     * True is eventually returned from execute() iff doBeforeAction returns true.
     * Set the event to something other than NONE if triggered.
     */
    bool execute( Event e=Event::NONE);

signals:
    void onEvent( EventGroup);   // Report to others that state changing event(s) have occurred.

protected:
    /**
     * Called on self at the end of init(). Override to manually adjust details of action/icon assignment here.
     */
    virtual void postInit() {}

    /**
     * Set asynchronous execution or not on the next call to execute().
     * Default is synchronous (blocking calls) in the GUI thread.
     * Normally, an asynchronous running action is not reentrant.
     * Set reentrant to true if async is also true to allow this action to
     * be triggered again even while executing inside its doAction function.
     */
    void setAsync( bool async, bool reentrant=false);
    bool isAsync() const { return _doasync;}
    bool isReentrant() const { return _reentrant;}

    /**
     * Returns the client set mouse position. Returns (-1,-1) if not set.
     * Always reset to (-1,-1) after doAfterAction executes.
     */
    const QPoint& primedMousePos() const { return _mpos;}

    /**
     * Derived actions may cache data against a model. This function is called to purge these data
     * because of events specified using addPurgeEvent or due to other conditions that can invalidate
     * data such as closing the model. The passed in event is the trigger for the purge call.
     */
    virtual void purge( const FM*, Event){}

    /**
     * First calls checkState on self and sets the action's checked state to the returned value.
     * Then calls checkEnable on self and sets the action's enabled state to the returned value.
     * This function is called at the end of this action being executed just prior to the
     * final onEvent signal being emitted. It can also be called internally at any point.
     */
    void refreshState( Event e=Event::NONE);

    /**
     * This function is called immediately before checkEnable. Derived actions should test the
     * application's state, configure themselves accordingly, and return whether they should be
     * checked or not (the returned value is used as the parameter to setChecked). The returned
     * value is ignored if this action is not checkable. The passed in Event is the reason for
     * calling this function.
     */
    virtual bool checkState( Event) { return isChecked();}

    /**
     * Called immediately after checkState, this function should return whether or not this action
     * should be enabled. Only enabled actions are available to the user and to be executed
     * in response to received events.
     */
    virtual bool checkEnable( Event) { return _unlocked;}

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
     * action is asynchronous.
     */
    virtual void doAfterAction( Event);

    /**
     * Override this function to provide custom recording of state for undo/redo functionality.
     * It is only necessary to implement this if storeUndo is called with autoRestore=false.
     * The returned UndoState object should contain sufficient data to perform restoreState with.
     * If the default implementation of UndoState does not have sufficient data fields, derive
     * a new type which does. Only the producing action will ever receive back the UndoState
     * objects that they create so it is always safe to downcast to the derived type.
     * To enable undo/redo functionality, the action should call UndoStates::storeUndo( this)
     * before carrying out modifications that should be allowed to be undone.
     */
    virtual UndoState::Ptr makeUndoState() const;   // Has default ERROR implementation!

    /**
     * Override this function to provide custom restoring of state for undo/redo functionality.
     * It is only necessary to implement this if storeUndo is called with autoRestore=false.
     * The action should restore state from an UndoState object. The passed in UndoState object
     * will have been previously created by this action's own overridden makeUndoState function
     * and so this action should use knowledge of the makeup of that object to restore state.
     * During this call, the model that was selected when makeUndoState was called will be locked
     * in write mode. After returning, the events originally passed to storeUndo are emitted and
     * the memory used by the UndoState object is reclaimed.
    */
    virtual void restoreState( const UndoState*);   // Has default ERROR implementation!

private slots:
    void endExecute( Event);

private:
    QAction _action;
    QString _dname;
    std::string _debugName;
    const QIcon _icon;
    const QKeySequence _keys;
    bool _init;
    bool _doasync;
    bool _reentrant;
    int _runCount;
    bool _reqConfirm;
    bool _unlocked; // If true, this action is enabled (true by default)
    Event _pevents; // Purge events
    Event _tevents; // Trigger events
    QPoint _mpos;   // The primed mouse position

    void pinit();

    /**
     * FaceActionManager calls init() on a FaceAction when being added to it. This sets up the internal QAction
     * instance and ensures that triggering the QAction causes the execute() function to run on this FaceAction.
     * It also adds the text to the action as getDisplayName(). If unhappy with any of this, make adjustments in
     * postInit() which is called at the end of init().
     */
    void init( QWidget* parent=nullptr);

    /**
     * Returns true if this action is currently within the doAction() function
     * (which may be called from a separate thread). FaceActionManager uses this
     * to determine which actions can be set ready. Running actions refresh themselves
     * immediately after doAfterAction() returns and before the final onEvent is emitted.
     */
    bool isRunning() const { return _runCount > 0;}

    friend class FaceActionWorker;
    friend class FaceActionManager;
    friend class UndoStates;
    friend class UndoState;
    friend class Interactor::ContextMenu;
    FaceAction( const FaceAction&) = delete;
    void operator=( const FaceAction&) = delete;
};  // end class

}}   // end namespaces

#endif
