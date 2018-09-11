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

#ifndef FACE_TOOLS_ACTION_FACE_ACTION_H
#define FACE_TOOLS_ACTION_FACE_ACTION_H

#include "EventProcessResponse.h"
#include <ModelViewerInteractor.h>
#include <PluginInterface.h>    // QTools
#include <QProgressUpdater.h>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QMutex>

namespace FaceTools {
namespace Action {

// Must remain pure virtual so Qt can make it a plugin interface.
class FaceTools_EXPORT FaceActionInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceActionInterface(){}
};  // end class


// See FaceActionManager for further information on how to correctly descend from FaceAction
// to ensure that application actions are able to respond to one another.
class FaceTools_EXPORT FaceAction : public FaceActionInterface
{ Q_OBJECT
public:
    FaceAction();

    // Constructor versions that take a display name, icon, and triggering key sequence.
    FaceAction( const QString& displayName);
    FaceAction( const QString& displayName, const QIcon& icon);
    FaceAction( const QString& displayName, const QIcon* icon);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence&);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence&);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence*);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence*);

    std::string dname() const { return "\"" + getDisplayName().remove('&').toStdString() + "\"";}

    QString getDisplayName() const override;
    const QIcon* getIcon() const override;
    const QKeySequence* getShortcut() const override;

    // Return this action's interactor (if defined). Only a single interactor per action is allowed.
    virtual Interactor::MVI* interactor() { return nullptr;}

    // Returns the internal QAction.
    // Triggering the action calls this action's process function.
    QAction* qaction() { return &_action;}

    // Visibility does NOT use QAction::visible because changing the QAction's
    // visibility to false also causes its enabled status to be false!
    void setVisible( bool b) { _visible = b;}
    bool isVisible() const { return _visible;}

    // If set false, this action won't be placed onto any toolbars. True by default.
    void setAllowOnToolbar( bool b) { _allowOnToolbar = b;}
    bool allowOnToolbar() const { return _allowOnToolbar;}

    // WARNING: Manually setting this action to be enabled when not allowed to set
    // its own enabled state (through calls to setReady and testSetEnabled) may cause
    // this action to fail on operation!
    void setEnabled( bool b);   // Also enabled/disables associated widget (if any).
    inline bool isEnabled() const { return _action.isEnabled();}

    void setCheckable( bool b, bool defaultCheckState);
    bool isCheckable() const { return _action.isCheckable();}
    inline bool defaultCheckState() const { return _defaultCheckState;}

    inline void setChecked( bool b) { _action.setChecked(b);}
    inline bool isChecked() const { return _action.isChecked();}

    // Sets the Vis::FV's this action will work over in the next call to process().
    // Each Vis::FV is first passed through this child class's implementation of testReady and
    // so only Vis::FV instances that adhere to the ready requirements of this action are admitted.
    void setReady( Vis::FV*, bool set=true);
    void setReady( const FVS&, bool set=true);

    void clearReady();  // Clear all entries from the ready set
    void resetReady( Vis::FV*); // Replace the ready set with the single view.

    inline bool isReady( const Vis::FV* fv) const { return _ready.has(fv);}
    inline bool gotReady() const { return !_ready.empty();}
    inline const FVS& ready() const { return _ready;}
    // Returns non-null only if the ready set has exactly 1 member (which is returned).
    inline const Vis::FV* ready1() const { return _ready.size() == 1 ? _ready.first() : nullptr;}

    // Test if this action should be enabled and set accordingly.
    // Clients can call with a non null point (representing the current mouse position)
    // for relevant actions to check. Internally when called, this function is always
    // called with null as its parameter. Returns whether or not the action is enabled as a result.
    bool testSetEnabled( const QPoint* mousePoint=nullptr);

    // Derived types and/or delegates can specify the events that they wish to be purged for and/or respond to
    // via their process() function. Purging (if specified for an event) always comes before process, so if the
    // same event is set as both a purge and a process event, the action will always be processed after purging
    // for that event. Moreover, purging of ALL actions is completed before processing of any responding actions starts.
    void setPurgeOnEvent( EventId);
    // Returns true iff the given event will cause this action to be purged on its receipt.
    inline bool isPurgeEvent( EventId e) const { return _pevents.count(e) > 0;}

    // This action's process function will be called in response to events set using the setRespondToEvent
    // and setRespondToEventIf functions. By default, an action is not set to respond to any event unless
    // set to do so either internally or externally. Parameter processFlag is passed to this action's process
    // function if a response can be granted to a received event. Alternatively, a delegate predicte can be used
    // to determine the process flag dynamically.
    void setRespondToEvent( EventId, bool processFlag=true);
    void setRespondToEvent( EventId, const ProcessFlagPredicate&);

    // Use setRespondToEventIf if the calling of this action's process function should depend both on
    // the event being received and the given predicate returning true. If this action is set to respond
    // to more than one event, this action's process function be called if any of the corresponding
    // response predicates return true for a given set of events.
    void setRespondToEventIf( EventId, const ResponsePredicate&, bool processFlag);
    void setRespondToEventIf( EventId, const ResponsePredicate&, const ProcessFlagPredicate&);

    // setRespondToEventIfAllReady uses predicate function std::all_of over the ready set to determine response criteria.
    void setRespondToEventIfAllReady( EventId, bool processFlag);

    // Returns non-null iff the given event is in this action's response set.
    inline const EPR* eventResponse( EventId e) const { return _eprs.count(e) > 0 ? &_eprs.at(e) : nullptr;}

    bool operator()( bool checkAction=true);  // Synonymous with process (see below).

signals:
    void reportStarting();   // Emitted from the GUI thread immediately before doBeforeAction executes.

    // Emitted from the GUI thread after doAfterAction returns (or after doBeforeAction returns false).
    void reportFinished( EventSet, FVS, bool);


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
    // 5) reportFinished
    // Calls 3, 4 and 5 are only made if doBeforeAction returns true. If doBeforeAction returns false,
    // signal reportFinished will be emitted. The return value of doAction is passed to doAfterAction.
    // True is returned from process() iff doAction was entered.
    bool process( bool checkAction=true);

    // These two versions of process replace the ready set with the given Vis::FVs and execute process.
    // On return, the action's ready set will be reset to the current set of selected Vis::FVs.
    bool process( Vis::FV*, bool checkAction=true);
    bool process( const FVS&, bool checkAction=true);

protected slots:
    /***************************************************************/
    /************** CONFIGURATION / GENERAL OPERATION **************/
    /***************************************************************/

    // FaceActionManager calls init() on a FaceAction when being added to it. This sets up the internal QAction
    // instance and ensures that triggering the QAction causes the process() function to run on this FaceAction.
    // It also adds the text to the action as getDisplayName(). If unhappy with any of this, make adjustments in
    // postInit() which is called at the end of init().
    void init();

    // Called on self at the end of init(). Override to manually adjust details of action/icon assignment here.
    virtual void postInit() {}

    // Set asynchronous execution or not on the next call to process. Default is synchronous (blocking calls).
    // Optionally set a progress updater which must be updated by derived type's implementation of doAction();
    // derived type should make regular calls to progress().
    void setAsync( bool, QTools::QProgressUpdater::Ptr pupdater=QTools::QProgressUpdater::Ptr());

    bool isAsync() const { return _doasync;}

    // If wanting to provide progress updates for long running actions that may be asynchronous, derived
    // type should regularly call this function to provide progress updates. This function first checks
    // to see if the external progress updater has been set (in setAsync) so is always safe to call.
    void progress( float propComplete);
    float progress() const; // Get the current progress


    /***************************************************************/
    /************** SETTING / GETTING DATA TO ACT UPON *************/
    /***************************************************************/

    // Function testReady is called to test whether the passed in Vis::FV is in an appropriate state for
    // this action to be executed on. This function is called whenever the user selects or deselects a view.
    // By default, all non-null Vis::FV instances are added to the ready set.
    virtual bool testReady( const Vis::FV *fv) { return fv != nullptr;}

    // Function tellReady is called when the given Vis::FV changes its ready state.
    // It is always called after setEnabled(testEnabled()).
    virtual void tellReady( Vis::FV*, bool isReady) {}

    // If this action is currently applied to the passed in Vis::FV, it should return true.
    // By default, this function just returns the current checked state meaning no changes are made.
    virtual bool testIfCheck( const Vis::FV *fv=nullptr) const { return isChecked();}

    // For most actions, whether to enable or disable the action depends upon whether there are
    // entries in the ready set and this is the default implementation. However, some actions may
    // want to provide a condition that tells whether this action should be enabled or disabled
    // for some other reason - for example for the action to be always enabled, override this
    // function to true. Or if the action should only be enabled if a single Vis::FV is
    // selected, override to return (_ready.size() == 1). Finally, the passed in mouse point
    // may not be null and in those cases the action may want to check if the position of the
    // mouse is important in determining whether the action should be enabled.
    virtual bool testEnabled( const QPoint* mouseCoords=nullptr) const { return gotReady();}

    // Derived types may cache data for FMs even when not selected. This function is
    // called to purge these cached data because of events specified using setPurgeOnEvent().
    virtual void purge( const FM*){}

    // Always called whenever a FM is closed. Some actions may want to store information
    // associated with a FM even when the action is being purged for the FM for other reasons
    // (specified by setPurgeOnEvent). This allows actions to hang on to data associated
    // with a FM until it's closed.
    virtual void clean( const FM* fm){ purge(fm);}


    /**************************************************************/
    /********************** ACTION DELEGATES **********************/
    /**************************************************************/

    // doBeforeAction always occurs in the GUI thread so this is where to show dialogs etc in order to get
    // user input. It is executed immediately before doAction which is only entered if the return value
    // from doBeforeAction is true. The ready set is passed in as non-const to allow the FaceAction to
    // change its membership prior to it being passed to the doAction function (e.g. to remove Vis::FV
    // instances that aren't suitable on further inspection, or the user wants to cancel actioning on).
    // Note that doAction may run in a different thread than the GUI thread.
    virtual bool doBeforeAction( FVS&, const QPoint&){ return true;}

    // Implement the action; process() decides whether it runs asynchronously or not (or at all).
    // If doAction runs asynchronously, defer all GUI updates etc to doAfterAction(). Membership
    // of the provided FVS may be changed within this function for passing to the
    // doAfterAction function in the GUI thread afterwards.
    virtual bool doAction( FVS&, const QPoint&){ return true;}

    // Called within the GUI thread immediately on the completion of doAction. This is where GUI
    // elements (dialogs etc) shown in doBeforeAction should be hidden or rendering updates made.
    // The ready set that was passed in to doAction is passed as a parameter. The EventSet
    // should be set with the events made.
    virtual void doAfterAction( EventSet&, const FVS&, bool){}


private slots:
    void doOnActionFinished( bool);

private:
    const QString _dname;
    const QIcon _icon;
    const QKeySequence _keys;
    bool _init;
    bool _visible;
    bool _defaultCheckState;
    bool _allowOnToolbar;
    QAction _action;
    bool _doasync;
    QMutex _pmutex;
    QTools::QProgressUpdater::Ptr _pupdater;
    FVS _ready, _wset;
    QPoint _testPoint;
    EventSet _pevents;  // Purge events
    std::unordered_map<EventId, EPR> _eprs;  // How to respond to events

    void checkOverwritingResponseEvent( EventId) const;
    friend class FaceActionManager;
    friend class FaceActionWorker;
    FaceAction( const FaceAction&) = delete;
    void operator=( const FaceAction&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#endif
