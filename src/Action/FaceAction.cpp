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

#include <FaceAction.h>
#include <FaceActionWorker.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::FaceActionWorker;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::FaceModel;
using FaceTools::Action::ChangeEvent;
using FaceTools::Action::ChangeEventSet;
using QTools::QProgressUpdater;


// public
FaceAction::FaceAction( bool dbo)
    : _dname(""), _icon(),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, bool dbo)
    : _dname(dname), _icon(), _keys(),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, bool dbo)
    : _dname(dname), _icon(ico), _keys(),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, bool dbo)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence& ks, bool dbo)
    : _dname(dname), _icon(ico), _keys(ks),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) {}   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence& ks, bool dbo)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) {}   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence* ks, bool dbo)
    : _dname(dname), _icon(ico), _keys(ks ? *ks : QKeySequence()),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) {}   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence* ks, bool dbo)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks ? *ks : QKeySequence()),
      _init(false), _disableBeforeOther(dbo), _externalSelect(true), _reportChanges(true),
      _action(this), _doasync(false), _pupdater(NULL) {}   // end ctor


// protected
void FaceAction::setExternalSelect( bool v) { _externalSelect = v;}


// public
QString FaceAction::getDisplayName() const { return _dname.isEmpty() ? QTools::PluginInterface::getDisplayName() : _dname; }


// public
const QIcon* FaceAction::getIcon() const { return _icon.isNull() ? NULL : &_icon;}


// public
const QKeySequence* FaceAction::getShortcut() const { return _keys.isEmpty() ? NULL : &_keys;}


// public
void FaceAction::connectToolButton( QToolButton* tb)
{
    tb->setDefaultAction( &_action);
    tb->setIconSize( QSize(26,26));
    tb->setMinimumSize( QSize(26,26));
    tb->setStyleSheet( "QToolButton::menu-indicator { image: none; }"); // Remove the menu-indicator
}   // end connectToolButton


// public
void FaceAction::connectButton( QPushButton* b)
{
    b->addAction( &_action);
    b->setIcon( _action.icon());
    b->setIconSize( QSize(26,26));
    b->setMinimumSize( QSize(26,26));
    b->connect( b, &QPushButton::clicked, &_action, &QAction::triggered);
}   // end connectButton


// public
QAction* FaceAction::qaction()
{
    if ( !_init)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::qaction: FaceAction::init() not called!" << std::endl;
        assert(_init);
        return NULL;
    }   // end if
    return &_action;
}   // end qaction


// protected
void FaceAction::init() // Called by FaceActionManager
{
    connect( &_action, &QAction::triggered, this, static_cast<bool (FaceAction::*)(bool)>(&FaceAction::process));
    _action.setText( getDisplayName());
    _action.setToolTip( getToolTip());
    const QIcon* icon = getIcon();
    if ( icon)
        _action.setIcon(*icon);
    const QKeySequence* keys = getShortcut();
    if ( keys)
        _action.setShortcut(*keys);
    setEnabled( testEnabled());
    _init = true;
}   // end init


// protected
void FaceAction::setAsync( bool enable, QProgressUpdater::Ptr pupdater)
{
    _doasync = enable;
    _pupdater = pupdater;
    if ( !isAsync()) // Ignore the progress updater if not async
        _pupdater = NULL;
}   // end setAsync


// protected
void FaceAction::progress( float propComplete)
{
    if (_pupdater)
        _pupdater->processUpdate( propComplete);
}   // end progress


// protected
void FaceAction::setSelected( FaceControl* fc, bool v)
{
    assert(fc);
    assert(fc->viewer());

    setChecked( testChecked(fc));

    if ( v == _ready.has(fc))   // Don't adjust the ready set unless a change will be made
        return;

    _ready.erase(fc);
    if ( v && testReady(fc))
        _ready.insert(fc);

    setEnabled( testEnabled());

    this->tellSelected( fc, v);
    this->tellReady( fc, _ready.has(fc));
}   // end setSelected


// public
void FaceAction::addChangeTo( const ChangeEvent& c) { _cevents.insert(c);}
void FaceAction::addRespondTo( const ChangeEvent& r) { _revents.insert(r);}
bool FaceAction::operator()(){ return process();}


// public
void FaceAction::execAfter( FaceAction* fa)
{
    assert(fa);
    if ( fa->isAsync())
        _aacts.insert(fa);
    else
        _sacts.push_back(fa);
}   // end execAfter


// protected
void FaceAction::respondTo( const FaceAction*, const ChangeEventSet*, FaceControl* fc)
{
    if ( !fc)
        return;
    if ( testReady(fc))
        _ready.insert(fc);
    else
        _ready.erase(fc);
    setEnabled( testEnabled());
}   // end respondTo


// public slot
bool FaceAction::process( bool checked)
{
    _wmutex.lock();
    assert(_init);
    setChecked(checked);
    const std::string dname = debugActionName();
    if ( displayDebugStatusProgression())
    {
        std::cerr << std::endl;
        std::cerr << "=== " << dname << " === STARTING" << std::endl;
    }   // end if

    bool rv = true;
    setEnabled(false);
    emit reportStarting( &_ready);
    if ( !doBeforeAction( _ready))  // Always in the GUI thread
    {
        if ( displayDebugStatusProgression())
            std::cerr << "=== " << dname << " === CANCELLED" << std::endl;
        setEnabled( testEnabled());
        emit reportFinished();
        rv = false;
        _wmutex.unlock();
    }   // end if
    else
    {
        if ( !isAsync())
        {
            if ( displayDebugStatusProgression())
                std::cerr << "--- " << dname << " --- WORKING" << std::endl;
            const bool rval = doAction( _ready);  // Blocks - doAction may change membership of _ready
            doOnActionFinished( rval);
        }   // end if
        else
        {
            if ( displayDebugStatusProgression())
                std::cerr << "--- " << dname << " --- WORKING (BG)" << std::endl;
            FaceActionWorker *worker = new FaceActionWorker( this, &_ready);
            connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::doOnActionFinished);
            connect( worker, &FaceActionWorker::finished, worker, &QObject::deleteLater);
            worker->start();   // Asynchronous start
        }   // end else
    }   // end else

    return rv;
}   // end process


// public
bool FaceAction::process( const FaceControlSet& cset)
{ 
    _ready.clear(); // Reset the ready set to that given
    for ( FaceControl* fc : cset)
    {
        if ( testReady(fc))
            _ready.insert(fc);
    }   // end for
    return process();
}   // end process


// public
bool FaceAction::process( FaceControl* fc)
{
    FaceControlSet fcs;
    fcs.insert(fc);
    return process( fcs);
}   // end process


// private
bool FaceAction::checkSequenceActions( std::unordered_set<const FaceAction*>& acts) const
{
    // If this action is already in the given set, the sequence of actions to be executed
    // must have cycles which is not allowed.
    if ( acts.count(this) > 0)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::checkSequenceActions: "
                  << "Sequence of actions to be executed contains more than one instance of "
                  << debugActionName() << std::endl;
        return false;
    }   // end if

    acts.insert(this);
    bool rv = true;
    for ( FaceAction* a : _sacts) // Consolidate sequence actions from this action.
    {
        rv = a->checkSequenceActions( acts);
        if ( !rv)
            break;
    }   // end for
    return rv;
}   // end checkSequenceActions


// private
void FaceAction::collectChangeEvents( ChangeEventSet& ces)
{
    assert( _reportChanges);
    _reportChanges = false;
    ces.insert( _cevents.begin(), _cevents.end());
    for ( FaceAction* a : _sacts)   // Collect change events (and set reporting to false) on child actions.
        a->collectChangeEvents( ces);
}   // end collectChangeEvents


// private slot
void FaceAction::doOnActionFinished( bool rval)
{
    doAfterAction( _ready, rval);   // _ready membership may have changed

    // Start asynchronous actions. Since these will end at undefined times (notifying others of their
    // completion via FaceActionManager), their change events are not consolidated into this action's.
    for ( FaceAction* a : _aacts)
        a->process(_ready);

#ifndef NDEBUG
    // Check if there a duplicate FaceAction instances anywhere in the sequential execution tree.
    std::unordered_set<const FaceAction*> allacts;
    bool checkOkay = checkSequenceActions( allacts);
    assert( checkOkay);
#endif

    // Synchronous actions will queue their reports of change events, so if there are multiple instances
    // of the same change event, it is only necessary to report it once and at the end of the sequence of
    // follow-on actions. Function collectChangeEvents collects the change events from the sequence of
    // actions and sets them all to refrain from reporting their completed actions.
    ChangeEventSet ces;
    if ( _reportChanges)
    {
        ces = changeEvents(); // Copy out
        for ( FaceAction* a : _sacts)
            a->collectChangeEvents( ces);   // Also sets _reportChanges to false
    }   // end if

    for ( FaceAction* a : _sacts)
        a->process(_ready);

    // ces won't be empty only if this is the 'parent' action.
    if ( !ces.empty())
    {
        for ( FaceControl* fc : _ready) // Notify other actions of the changes via the FaceActionManager.
            emit reportChanges( ces, fc);
    }   // end if

    _reportChanges = true;

    progress(1.0f);
    if ( displayDebugStatusProgression())
        std::cerr << "=== " << debugActionName() << " === FINISHED" << std::endl;

    _wmutex.unlock();
    emit reportFinished();
}   // end doOnActionFinished


// protected (default)
void FaceAction::doAfterAction( const FaceControlSet& fst, bool)
{
    const FaceModelSet& fms = fst.models(); // The models
    FaceViewerSet vws;
    // Models with a view update flagged need the views updating across all their associated FaceControls.
    for ( FaceModel* fm : fms)
    {
        if ( fm->_flagViewUpdate)
        {
            std::for_each( std::begin( fm->_fcs), std::end( fm->_fcs),
                    [&](FaceControl* fc){
                        fc->view()->rebuild();      // Rebuild views
                        vws.insert(fc->viewer());   // Ensure reference to viewer is collected for updateRender
                        });
            fm->_flagViewUpdate = false;
        }   // end if
    }   // end for

    // Update rendering on all viewers.
    if (vws.empty())
        vws = fst.viewers();
    std::for_each( std::begin(vws), std::end(vws), [](auto v){ v->updateRender();});
}   // end doAfterAction
