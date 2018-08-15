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
#include <ModelViewerInteractor.h>
#include <FaceControlSet.h>
#include <PluginInterface.h>    // QTools
#include <QProgressUpdater.h>
#include <QToolBar>
#include <QToolButton>
#include <QPushButton>
#include <QAction>
#include <QString>
#include <QMutex>

namespace FaceTools {
namespace Action {

// Must remain pure virtual so Qt can make it a plugin interface.
class FaceTools_EXPORT FaceActionInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual ~FaceActionInterface(){}
};  // end class


class ActionVisualise;
class FaceActionWorker;
class FaceActionManager;


// See FaceActionManager for further information on how to correctly descend from FaceAction
// to ensure that application actions are able to respond to one another.
class FaceTools_EXPORT FaceAction : public FaceActionInterface
{ Q_OBJECT
public:
    FaceAction();
    ~FaceAction() override;

    // Constructor versions that take a display name, icon, and triggering key sequence.
    FaceAction( const QString& displayName);
    FaceAction( const QString& displayName, const QIcon& icon);
    FaceAction( const QString& displayName, const QIcon* icon);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence&);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence&);
    FaceAction( const QString& displayName, const QIcon& icon, const QKeySequence*);
    FaceAction( const QString& displayName, const QIcon* icon, const QKeySequence*);

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
    inline void setVisible( bool b) { _visible = b;}
    inline bool isVisible() const { return _visible;}

    void setEnabled( bool b);   // Also enabled/disables associated widget (if any).
    inline bool isEnabled() const { return _action.isEnabled();}

    inline void setCheckable( bool b, bool ival) { _action.setCheckable(b); _action.setChecked(ival);}
    inline bool isCheckable() const { return _action.isCheckable();}

    inline void setChecked( bool b) { _action.setChecked(b);}
    inline bool isChecked() const { return _action.isChecked();}

    // Sets the FaceControl's this action will work over in the next call to process().
    // Each FaceControl is first passed through this child class's implementation of testReady and
    // so only FaceControl instances that adhere to the ready requirements of this action are admitted.
    void setReady( FaceControl*, bool set=true);
    void setReady( const FaceControlSet&, bool set=true);

    // Test if this action should be enabled and set accordingly.
    // Clients can call with a non null point (representing the current mouse position)
    // for relevant actions to check. Internally when called, this function is always
    // called with null as its parameter. Returns whether or not the action is enabled as a result.
    bool testSetEnabled( const QPoint* mousePoint=nullptr);

    // Derived types and/or delegates must specify the ChangeEvents that they wish to be processed and purged for.
    // Purging the action (if necessary) always comes before process, so if the same ChangeEvent is set as
    // both a purge and a process event, the action will always be processed after purging for that event.
    // Moreover, purging of all actions is completed before processing of any responding actions starts.
    void addPurgeOn( const ChangeEvent&);
    void addProcessOn( const ChangeEvent&);
    const ChangeEventSet& purgeEvents() const { return _pevents;}   // Changes this Action will be purged for.
    const ChangeEventSet& processEvents() const { return _revents;} // Changes this Action will be processed for.

    bool operator()( bool checkAction=true);  // Synonymous with process (see below).

signals:
    void reportStarting();   // Emitted from the GUI thread immediately before doBeforeAction executes.

    // Emitted from the GUI thread after doAfterAction returns (or after doBeforeAction returns false).
    void reportFinished( ChangeEventSet, FaceControlSet, bool);


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

    // These two versions of process replace the ready set with the given FaceControls and execute process.
    // On return, the action's ready set will be reset to the current set of selected FaceControls.
    bool process( FaceControl*, bool checkAction=true);
    bool process( const FaceControlSet&, bool checkAction=true);

    // Add composite actions to this one that will be executed immediately after doAfterAction() returns.
    // This function can be called multiple times to add several actions. Asynchronous actions are executed
    // first (in no definite order) followed by the non-asynchronous actions which are executed sequentially
    // in their order of addition. Only FaceAction instances that haven't already been added to this FaceAction
    // (or added to any of the other already added FaceAction instances) can be added. Self adding is also
    // not allowed. Returns true on the successful addition of the FaceAction.
    bool execAfter( FaceAction*);

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
    float progress() const; // Get the current progress


    /***************************************************************/
    /************** SETTING / GETTING DATA TO ACT UPON *************/
    /***************************************************************/

    // Function testReady is called to test whether the passed in FaceControl is in an appropriate state for
    // this action to be executed on. This function is called whenever the user selects or deselects a view.
    // By default, all non-null FaceControl instances are added to the ready set.
    virtual bool testReady( const FaceControl *fc) { return fc != nullptr;}

    // Function tellReady is called when the given FaceControl changes its ready state.
    // It is always called after setEnabled(testEnabled()).
    virtual void tellReady( FaceControl*, bool isReady) {}

    // If this action is currently applied to the passed in FaceControl, it should return true.
    // By default, this function just returns the current checked state meaning no changes are made.
    virtual bool testChecked( const FaceControl *fc=nullptr) const { return isChecked();}

    // Reset the ready set with the given FaceControl if it passes the testReady check (above).
    bool isReady( const FaceControl* fc) const { return _ready.has(fc);}
    bool gotReady() const { return !_ready.empty();}
    const FaceControlSet& ready() const { return _ready;}
    // Returns non-null only if the ready set has exactly 1 member (which is returned).
    FaceControl* ready1() const { return _ready.size() == 1 ? _ready.first() : nullptr;}

    // For most actions, whether to enable or disable the action depends upon whether there are
    // entries in the ready set and this is the default implementation. However, some actions may
    // want to provide a condition that tells whether this action should be enabled or disabled
    // for some other reason - for example for the action to be always enabled, override this
    // function to true. Or if the action should only be enabled if a single FaceControl is
    // selected, override to return (_ready.size() == 1). Finally, the passed in mouse point
    // may not be null and in those cases the action may want to check if the position of the
    // mouse is important in determining whether the action should be enabled.
    virtual bool testEnabled( const QPoint* mouseCoords=nullptr) const { return gotReady();}

    // Derived types may cache data for FaceModels even when not selected. This function is
    // called to purge these cached data because of ChangeEvents specified using addPurgeOn().
    virtual void purge( const FaceModel*){}

    // Always called whenever a FaceModel is closed. Some actions may want to store information
    // associated with a FaceModel even when the action is being purged for the FaceModel for
    // other reasons (specified by addPurgeOn). This allows actions to hang on to data associated
    // with a FaceModel until it's closed.
    virtual void clean( const FaceModel* fm){ purge(fm);}


    /**************************************************************/
    /********************** ACTION DELEGATES **********************/
    /**************************************************************/

    // doBeforeAction always occurs in the GUI thread so this is where to show dialogs etc in order to get
    // user input. It is executed immediately before doAction which is only entered if the return value
    // from doBeforeAction is true. The ready set is passed in as non-const to allow the FaceAction to
    // change its membership prior to it being passed to the doAction function (e.g. to remove FaceControl
    // instances that aren't suitable on further inspection, or the user wants to cancel actioning on).
    // Note that doAction may run in a different thread than the GUI thread.
    virtual bool doBeforeAction( FaceControlSet&, const QPoint&){ return true;}

    // Implement the action; process() decides whether it runs asynchronously or not (or at all).
    // If doAction runs asynchronously, defer all GUI updates etc to doAfterAction(). Membership
    // of the provided FaceControlSet may be changed within this function for passing to the
    // doAfterAction function in the GUI thread afterwards.
    virtual bool doAction( FaceControlSet&, const QPoint&){ return true;}

    // Called within the GUI thread immediately on the completion of doAction. This is where GUI
    // elements (dialogs etc) shown in doBeforeAction should be hidden or rendering updates made.
    // The ready set that was passed in to doAction is passed as a parameter. The ChangeEventSet
    // should be set with the ChangeEvents made.
    virtual void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool){}


    /**************************************************************/
    /****************** UTILITY / MISCELLANEOUS *******************/
    /**************************************************************/

    std::string debugActionName() const { return "\"" + getDisplayName().remove('&').toStdString() + "\"";}

    // For simple actions, may want to override this to false.
    virtual bool displayDebugStatusProgression() const { return true;}
    friend class FaceActionWorker;

private slots:
    void doOnActionFinished( bool);

private:
    const QString _dname;
    const QIcon _icon;
    const QKeySequence _keys;
    bool _init;
    bool _visible;
    QAction _action;
    bool _doasync;
    ChangeEventSet* _cset;
    QMutex _pmutex;
    QTools::QProgressUpdater::Ptr _pupdater;
    FaceControlSet _ready, _wset;
    QPoint _testPoint;
    ChangeEventSet _revents, _pevents;
    std::list<FaceAction*> _sacts;
    std::unordered_set<FaceAction*> _aacts;

    bool checkAfterActions( std::unordered_set<FaceAction*>&, FaceAction*) const;
    void setChangeEventSet( ChangeEventSet*);

    friend class FaceActionManager;

    FaceAction( const FaceAction&) = delete;
    void operator=( const FaceAction&) = delete;
};  // end class

}   // end namespace
}   // end namespace

#define FaceToolsPluginFaceActionInterface_iid "com.github.richeytastic.FaceTools.v030.FaceActionInterface"
Q_DECLARE_INTERFACE( FaceTools::Action::FaceActionInterface, FaceToolsPluginFaceActionInterface_iid)

#endif
