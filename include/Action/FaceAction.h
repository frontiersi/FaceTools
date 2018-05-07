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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_H
#define FACE_TOOLS_ACTION_FACE_ACTION_H

#include "ChangeEvents.h"
#include <FaceControl.h>
#include <FaceControlSet.h>
#include <PluginInterface.h>    // QTools
#include <QProgressUpdater.h>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QString>

namespace FaceTools {
namespace Action {

// Must remain pure virtual so Qt can make it a plugin interface.
class FaceTools_EXPORT FaceActionInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceActionInterface(){}
};  // end class


class FaceActionWorker;
class FaceActionManager;


// See FaceActionManager for further information on how to correctly descend from FaceAction
// to ensure that application actions are able to respond to one another.
class FaceTools_EXPORT FaceAction : public FaceActionInterface
{ Q_OBJECT
public:
    // Assuming this FaceAction is registered with the FaceActionManager, by setting disableBeforeOther to
    // true, this action will disable itself (setEnabled(false)) prior to the execution of any other action.
    explicit FaceAction( bool disableBeforeOther=false);
    ~FaceAction() override {}

    // Connect the provided buttons to this action.
    void connectToolButton( QToolButton*);
    void connectButton( QPushButton*);

    // Returns the QAction as long as this action had init() called on it by child class.
    // Triggering the action calls this action's process function.
    QAction* qaction();

    bool operator()();  // Synonymous with process (see below).

    // Derived types and/or delegates must specify what kinds of changes they make and what kinds of changes
    // they're interested in. This is necessary for FaceActionManager to connect reportChanged signals with
    // the corresponding respondToChange slots.
    void addChangeTo( const ChangeEvent&);   // Call with the changes they make
    void addRespondTo( const ChangeEvent&);  // Call with the changes they care about

    // Function changeEvents() gives clients the set of change events that this action performs.
    // FaceActionManager uses this info to connect the reportChanged signal on this FaceAction to
    // the respondToChange slot on other FaceAction instances that indicate their interest in
    // such changes via the ChangeEventSet returned from the respondEvents() function.
    // See ChangeEvents.h for details of event types.
    const ChangeEventSet& changeEvents() const { return _cevents;}

    // Function respondEvents() allows clients to be informed of the set of events that this FaceAction
    // is interested in responding to. FaceActionManager uses this info to connect this FaceAction's
    // respondToChange slot to the reportChanged signal emitted by other FaceAction instances that
    // cause these events to occur (detailed via the ChangeEventSet returned by changeEvents).
    // See ChangeEvents.h for details of event types.
    const ChangeEventSet& respondEvents() const { return _revents;}

public slots:
    // Function process() is the main entry point for clients to carry out this action. It can be called
    // explicitly whether or not this FaceAction is enabled, but is typically called via triggering the
    // internal enabled QAction returned from qaction(). The following sequence of calls are made:
    // 1) reportStarting
    // 2) doBeforeAction
    // 3) doAction
    // 4) doAfterAction
    // 5) reportChanged
    // 6) reportFinished
    // Calls 3,4, and 5 are only made if doBeforeAction returns true. If doBeforeAction returns false,
    // signal reportFinished will be emitted with a NULL parameter to denote that no FaceControl instances
    // were acted upon. If doBeforeAction returns true, and calls 3 and 4 are made and signal reportChanged
    // is emitted for every FaceControl acted upon and a non-NULL pointer to the worked over set of
    // FaceControl instances will be emitted by reportFinished. The return value of doAction is passed
    // to doAfterAction.
    // 
    // All calls except possibly doAction execute synchronously in Qt's GUI thread. Function doAction
    // may operate asynchronously in a different thread depending on whether the derived type set this
    // with a call to setAsync() (typically within its constructor).
    //
    // True is returned from process() iff doAction was entered.
    bool process();

    // Process the given set of FaceControl instances by first erasing this action's controlled and
    // ready sets and testing each member in the given set by called setSelected. This constructs
    // a new ready set which is a subset of the provided set. Following this, the process function
    // is immediately called to action these FaceControl instances according to this action's normal
    // logic. This function is provided to allow actions to chain themselves together. This function
    // should be called in a containing action's doAfterAction function. Note that processing of this
    // action will behave like an "inner" function of a composite function with the outer (calling)
    // action only returning from its doAfterAction function once this inner action has returned from
    // its own doAfterAction. Note that this also means that external actions will be notified of
    // the inner action finishing before the outer action finishes.
    bool chain( const FaceControlSet&);

signals:
    void reportStarting( const FaceControlSet*);   // Emitted immediately before doBeforeAction executes.

    // This signal is emitted to inform the FaceActionManager that this action has made changes to the given
    // FaceControl. It is emitted after doAfterAction returns but before reportFinished is emitted.
    // FaceActionManager fowards this signal to all other FaceAction instances registered with it that are
    // interested in events performed by the emitting action. This allows "listening" actions that might be
    // responsible for maintaining state the ability to react to changes performed by other (unknown) actions
    // even if affected FaceControl instances aren't in the receiving action's "ready" set.
    void reportChanged( FaceControl*);

    void reportFinished( const FaceControlSet*);  // Emitted after doAfterAction returns (or after doBeforeAction->false)

protected slots:
    /***************************************************************/
    /************** CONFIGURATION / GENERAL OPERATION **************/
    /***************************************************************/

    // FaceActionManager calls init() on a FaceAction when being added to it. This sets up the internal QAction
    // instance and ensures that triggering the QAction causes the process() function to run on this FaceAction.
    void init();

    // Set asynchronous execution or not on the next call to process. Default is synchronous (blocking calls).
    // Optionally set a progress updater which must be updated by derived type's implementation of doAction();
    // derived type should make regular calls to progress().
    void setAsync( bool, QTools::QProgressUpdater* pupdater=NULL);

    // If wanting to provide progress updates for long running actions that may be asynchronous, derived
    // type should regularly call this function to provide progress updates. This function first checks
    // to see if the external progress updater has been set (in setAsync) so is always safe to call.
    void progress( float propComplete);


    /***************************************************************/
    /************** SETTING / GETTING DATA TO ACT UPON *************/
    /***************************************************************/

    // FaceActionManager calls to inform of the selection state for the given FaceControl instance. If called with
    // true as its parameter, testReady() is called to allow the derived type to check if the given FaceControl is
    // in a state that allows it to be acted upon. When doAction() is called, the parameter set passed in is the
    // subset of the controlled set where testReady() returned true.
    void setSelected( FaceControl*, bool);

    // Derived FaceAction types may be interested in all calls to setSelected, and not just calls to testReady
    // (which is used to find out if a FaceControl under nominal control can be acted upon by the derived type).
    // Functions tellSelected and tellReady are called for calls to setSelected that causes a change in membership
    // of the controlled or ready sets (as such, these functions are not called for every call to setSelected).
    // Function tellSelected passes the changed controlled state to the derived type, and tellReady passes the
    // changed ready state of the FaceControl to the derived type. These functions are called *after* setEnabled
    // is called on this action (i.e. testEnabled is called before tellReady).
    virtual void tellSelected( FaceControl*, bool) {}
    virtual void tellReady( FaceControl*, bool) {}

    // This function is called only if setSelected is called with true as its parameter. It is used to ask the
    // derived type if the passed in FaceControl is in a legal state to be acted upon. Note that this function
    // is NOT called if setSelected is called with false as its parameter (clients should override tellReady()
    // if they need to know when a FaceControl switches legal state for being acted upon).
    // Whether or not a FaceAction is ready be be acted upon is at the discretion of the derived type, but by
    // default all FaceControl instances passed in to setControlled with a true parameter are added to the
    // ready set. If overridden, this function's complexity should be low due to the frequent calls.
    virtual bool testReady( FaceControl*) { return true;}

    // Discover if the given FaceControl is currently under nominal control or is in the ready set.
    bool isSelected( FaceControl *fc) const { _controlled.count(fc) > 0;}
    bool isReady( FaceControl* fc) const { _ready.count(fc) > 0;}

    // Retain in the "ready" set only those FaceControl instances for which calls to testReady()
    // still return true, returning the size of the modified set and test-setting the enabled state of
    // this action. Unless testReady() overridden, ready set will equal the controlled set.
    size_t recheckReadySet();

    // Return the count of FaceControl instances in the ready set.
    size_t readyCount() const { return _ready.size();}

    // For most actions, whether to enable or disable the action depends upon whether there are
    // entries in the ready set and this is the default implementation. However, some actions may
    // want to provide a condition that tells whether this action should be enabled or disabled
    // for some other reason - for example for the action to be always enabled, override this
    // function to true. Or if the action should only be enabled if a single FaceControl is
    // selected, override to return (_ready.size() == 1) etc.
    virtual bool testEnabled() { return !_ready.empty();}

    // Derived types may cache data for FaceControl instances even when not selected. This function is called
    // when there is no chance of the FaceControl being selected again in this session (typically because the
    // model's been closed). It is only ever called after setSelected(f,false) for FaceControl f.
    virtual void burn( const FaceControl*){}


    /**************************************************************/
    /********************** ACTION DELEGATES **********************/
    /**************************************************************/

    // doBeforeAction always occurs in the GUI thread so this is where to show dialogs etc in order to get
    // user input. It is executed immediately before doAction which is only entered if the return value
    // from doBeforeAction is true. The ready set is passed in as non-const to allow the FaceAction to
    // change its membership prior to it being passed to the doAction function (e.g. to remove FaceControl
    // instances that aren't suitable on further inspection, or the user wants to cancel actioning on).
    // Note that doAction may run in a different thread than the GUI thread.
    virtual bool doBeforeAction( FaceControlSet&){ return true;}

    // Implement the action; process() decides whether it runs asynchronously or not (or at all).
    // If doAction runs asynchronously, defer all GUI updates etc to doAfterAction(). The ready set
    // is passed in as non-const to allow the derived type to update the set to comprise only those
    // FaceControl instances that were successfully acted upon.
    virtual bool doAction( FaceControlSet&){ return false;}

    // Called within the GUI thread immediately on the completion of doAction. This is where GUI
    // elements (dialogs etc) shown in doBeforeAction should be hidden or rendering updates made.
    // The default implementation is to update rendering on all the viewers referenced by the
    // provided FaceControl instances. Note that the ready set is passed in const (after membership
    // having been potentially changed in doAction).
    virtual void doAfterAction( const FaceControlSet&, bool);


    /**************************************************************/
    /**************** INTER-ACTION COMMUNICATION ******************/
    /**************************************************************/

    // Allow this FaceAction to respond after notification of changes to a FaceControl by a different
    // FaceAction once it's finished. FaceActionManager ensures an action's reportChanged signal is
    // connected to the respondToChange slot on all other actions that specify their interest in the
    // events changed by the reporting action. If not overridden, the default function is to check the
    // passed in FaceControl's ready state and then call testAndSetEnabled(). If a derived type overrides
    // this function to (for example) enforce state, it should normally call this function as the last
    // line in the body of the overridden function (i.e. FaceAction::respondToChange).
    virtual void respondToChange( FaceControl*);

    // Used by FaceActionManager to know if it should set this action to be disabled in response
    // to other FaceActions registered with it starting.
    bool isDisabledBeforeOther() const { return _disableBeforeOther;}


    /**************************************************************/
    /****************** UTILITY / MISCELLANEOUS *******************/
    /**************************************************************/

    void setVisible( bool b) { _action.setVisible(b);}
    void setEnabled( bool b) { _action.setEnabled(b);}
    void setCheckable( bool b, bool ival) { _action.setCheckable(b); _action.setChecked(ival);}
    bool isCheckable() const { return _action.isCheckable();}
    void setChecked( bool b) { _action.setChecked(b);}
    bool isChecked() const { return _action.isChecked();}
    bool isEnabled() const { return _action.isEnabled();}

    std::string debugActionName() const { return "\"" + getDisplayName().remove('&').toStdString() + "\"";}

    // For simple actions, may want to override this to false.
    virtual bool displayDebugStatusProgression() const { return true;}
    friend class FaceActionManager;
    friend class FaceActionWorker;

private slots:
    void doOnActionFinished( bool);

private:
    bool _init;
    bool _disableBeforeOther;
    QAction _action;
    bool _doasync;
    QTools::QProgressUpdater* _pupdater;
    FaceControlSet _controlled, _ready;
    ChangeEventSet _revents, _cevents;
    FaceControlSet _pready; // Pre-doAction caching of _ready

    FaceAction( const FaceAction&);     // No copy
    void operator=( const FaceAction&); // No copy
};  // end class

}   // end namespace
}   // end namespace

#define FaceToolsPluginFaceActionInterface_iid "com.github.richeytastic.FaceTools.v030.FaceActionInterface"
Q_DECLARE_INTERFACE( FaceTools::Action::FaceActionInterface, FaceToolsPluginFaceActionInterface_iid)

#endif
