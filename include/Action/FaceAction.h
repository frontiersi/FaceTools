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
#include <QMutex>
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
    // In general this should be true because other (unknown) actions may concurrently update the data this
    // action expects to be working on.
    explicit FaceAction( bool disableBeforeOther=true);

    // Constructor versions that take a display name, icon, and triggering key sequence.
    FaceAction( const QString& displayName, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon& icon, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon* icon, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence&, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence&, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence*, bool disableBeforeOther=true);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence*, bool disableBeforeOther=true);

    QString getDisplayName() const override;
    const QIcon* getIcon() const override;
    const QKeySequence* getShortcut() const override;

    // Returns the QAction as long as this action had init() called on it by child class.
    // Triggering the action calls this action's process function.
    QAction* qaction();

    // Utility functions to connect the provided buttons to this action.
    void connectToolButton( QToolButton*);
    void connectButton( QPushButton*);


    // Derived types and/or delegates must specify what kinds of changes they make and what kinds of changes
    // they're interested in.
    void addChangeTo( const ChangeEvent&);   // Call with the changes they make
    void addRespondTo( const ChangeEvent&);  // Call with the changes they care about

    // Function changeEvents() gives clients the set of change events that this action performs.
    // FaceActionManager uses this info to connect the reportChanges signal on this FaceAction to
    // the respondTo function on other FaceAction instances that indicate their interest in
    // such changes via the ChangeEventSet returned from the respondEvents() function.
    // See ChangeEvents.h for details of event types.
    const ChangeEventSet& changeEvents() const { return _cevents;}

    // Function respondEvents() allows clients to be informed of the set of events that this FaceAction
    // is interested in responding to. FaceActionManager uses this info to connect this FaceAction's
    // respondTo function to the reportChanges signal emitted by other FaceAction instances that
    // cause these events to occur (detailed via the ChangeEventSet returned by changeEvents).
    // See ChangeEvents.h for details of event types.
    const ChangeEventSet& respondEvents() const { return _revents;}


    bool operator()();  // Synonymous with process (see below).

public slots:
    // Function process() is the main entry point for clients to carry out this action. It can be called
    // explicitly whether or not this FaceAction is enabled, but is typically called via triggering the
    // internal enabled QAction returned from qaction(). When triggered, the checkAction parameter
    // takes on the value of QAction::isChecked. When calling externally, the check state of the QAction
    // will be set according to the parameter (true by default).
    // The following sequence of calls are made:
    // 1) reportStarting
    // 2) doBeforeAction
    // 3) doAction
    // 4) doAfterAction
    // 5) reportChanges
    // 6) reportFinished
    // Calls 3,4, and 5 are only made if doBeforeAction returns true. If doBeforeAction returns false,
    // signal reportFinished will be emitted. If doBeforeAction returns true, calls 3 and 4 are made and
    // signal reportChanges is emitted for every FaceControl acted upon before reportFinished is emitted.
    // The return value of doAction is passed to doAfterAction.
    // 
    // All calls except possibly doAction execute synchronously in Qt's GUI thread. Function doAction
    // may operate asynchronously in a different thread depending on whether the derived type set this
    // with a call to setAsync() (typically within its constructor).
    //
    // True is returned from process() iff doAction was entered.
    bool process( bool checkAction=true);

    // These two versions of process replace the ready set with the given FaceControls and execute process.
    // On return, the action's ready set will be reset to the current set of selected FaceControls.
    bool process( const FaceControlSet&);
    bool process( FaceControl*);

    // This function is used to add composite actions to this one that will be executed immediately after the
    // containing action's doAfterAction() function returns. This function can be called multiple times to add
    // several actions. Asynchronous actions are executed first (in no definite order) followed by the
    // non asynchronous actions which are executed sequentially in their order of addition.
    void execAfter( FaceAction*);

signals:
    void reportStarting( const FaceControlSet*);   // Emitted immediately before doBeforeAction executes.
    void reportChanges( const ChangeEventSet&, FaceControl*);   // Inform FaceActionManager of changes to given FaceControl.
    void reportFinished();  // Emitted after doAfterAction returns (or after doBeforeAction returns false).

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
    void setAsync( bool, QTools::QProgressUpdater::Ptr pupdater=QTools::QProgressUpdater::Ptr());

    bool isAsync() const { return _doasync;}

    // If wanting to provide progress updates for long running actions that may be asynchronous, derived
    // type should regularly call this function to provide progress updates. This function first checks
    // to see if the external progress updater has been set (in setAsync) so is always safe to call.
    void progress( float propComplete);


    /***************************************************************/
    /************** SETTING / GETTING DATA TO ACT UPON *************/
    /***************************************************************/

    // FaceActionManager calls setSelected to inform of the selection state for the given FaceControl instance.
    // It is the entry point for the FaceActionManager to cause this action to respond to FaceControl selection
    // events originating externally (e.g. from user interaction). This function allows the action to decide if
    // the FaceControl should be a part of its "ready" set. If externalSelect() returns false, this fuction will
    // not be called from external events and the action will be left up to itself to decide when this function
    // should be called on itself (perhaps due to some other events under its own control).
    // If called with true as its parameter, testReady() is called to allow the derived type to check if the
    // given FaceControl is in a state that allows it to be acted upon. When doAction() is called, the parameter
    // set passed in is the set of FaceControls for which testReady() returned true.
    virtual void setSelected( FaceControl* fc, bool v);

    // FaceAction instances can implement tellSelected to know when selection status changes on a FaceControl.
    // It is called at the end of setSelected. Similarly, function tellReady is called by this action at the end
    // of setSelected to inform derived types of a change in the "ready" status of a passed in FaceControl.
    // Both functions are called if setSelected is called - irrespective of whether the FaceControl is found
    // in the ready set afterwards. Note that testEnabled is called before tellReady.
    virtual void tellSelected( FaceControl*, bool) {}
    virtual void tellReady( FaceControl*, bool) {}

    // This function is called only if setSelected is called with true as its parameter. It is used to ask the
    // derived type if the passed in FaceControl is in a legal state to be acted upon. Note that this function
    // is NOT called if setSelected is called with false as its parameter (clients should override tellReady()
    // if they need to know when a FaceControl switches legal state for being acted upon).
    // Whether or not a FaceAction is ready be be acted upon is at the discretion of the derived type, but by
    // default all FaceControl instances passed in to setSelected with a true parameter are added to the
    // ready set. If overridden, this function's complexity should be low due to the frequent calls.
    virtual bool testReady( FaceControl*) { return true;}
    virtual bool testChecked( FaceControl*) { return true;}

    // Is FaceControl in the ready set?
    bool isReady( FaceControl* fc) const { return _ready.has(fc);}

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
    virtual void purge( const FaceControl*){}


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

    // Allow this FaceAction to respond to asynchronous notification of changes to a FaceControl by some
    // other FaceAction after it's finished. The "changing" action is passed in as parameter and the set
    // of change events passed in as second parameter. Note that changes are not guaranteed to have occurred -
    // they are just the aggregate set of changes that may possibly have occurred. If the action
    // that enacted the change is asynchronous (i.e. isAsync() == true), this will be the parameter action.
    // Non-asynchronous actions may be queued however and in this case, it is the "root" action that is passed
    // as the parameter to this function which may not be the action directly responsible for the change!
    // This function's default implementation ignores the FaceAction parameter, checks the ready state
    // and then calls setEnabled(testEnabled()). *** This function is reentrant! ***
    virtual void respondTo( const FaceAction*, const ChangeEventSet*, FaceControl *fc=NULL);

    // Used by FaceActionManager to know if it should set this action to be disabled in response
    // to other FaceActions registered with it starting.
    bool isDisabledBeforeOther() const { return _disableBeforeOther;}

    // Set/get whether this action can be setSelected() (see above) by external events managed through
    // the FaceActionManager. By default, this is true.
    void setExternalSelect( bool);
    bool externalSelect() const { return _externalSelect;}


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
    QString _dname;
    QIcon _icon;
    QKeySequence _keys;
    bool _init;
    bool _disableBeforeOther;
    bool _externalSelect;
    bool _reportChanges;
    QAction _action;
    bool _doasync;
    QMutex _wmutex;  // The "work" mutex
    QTools::QProgressUpdater::Ptr _pupdater;
    FaceControlSet _ready;
    ChangeEventSet _cevents;
    ChangeEventSet _revents;
    std::list<FaceAction*> _sacts;
    std::unordered_set<FaceAction*> _aacts;
    bool checkSequenceActions( std::unordered_set<const FaceAction*>&) const;
    void collectChangeEvents( ChangeEventSet&);
    FaceAction( const FaceAction&);     // No copy
    void operator=( const FaceAction&); // No copy
};  // end class

}   // end namespace
}   // end namespace

#define FaceToolsPluginFaceActionInterface_iid "com.github.richeytastic.FaceTools.v030.FaceActionInterface"
Q_DECLARE_INTERFACE( FaceTools::Action::FaceActionInterface, FaceToolsPluginFaceActionInterface_iid)

#endif
