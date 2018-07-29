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
#include <FaceControl.h>
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
FaceAction::FaceAction()
    : _dname(""), _icon(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname)
    : _dname(dname), _icon(), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico)
    : _dname(dname), _icon(ico), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence& ks)
    : _dname(dname), _icon(ico), _keys(ks),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence& ks)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence* ks)
    : _dname(dname), _icon(ico), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence* ks)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }   // end ctor


// public
QString FaceAction::getDisplayName() const { return _dname.isEmpty() ? QTools::PluginInterface::getDisplayName() : _dname; }
const QIcon* FaceAction::getIcon() const { return _icon.isNull() ? nullptr : &_icon;}
const QKeySequence* FaceAction::getShortcut() const { return _keys.isEmpty() ? nullptr : &_keys;}


// public
QAction* FaceAction::qaction()
{
    if ( !_init)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::qaction: init() not called!" << std::endl;
        assert(_init);
        return nullptr;
    }   // end if
    return &_action;
}   // end qaction


// public
bool FaceAction::testSetEnabled()
{
    const bool enable = testEnabled();
    setEnabled( enable);
    if ( !enable && isCheckable())
        setChecked(false);
    assert( isEnabled() == enable);
    return isEnabled();
}   // end testSetEnabled


// public
void FaceAction::setEnabled( bool v)
{
    _action.setEnabled(v);
    if ( getWidget())
        getWidget()->setEnabled(v);
}   // end setEnabled


// protected
void FaceAction::init() // Called by FaceActionManager after constructor finished
{
    connect( &_action, &QAction::triggered, this, static_cast<bool (FaceAction::*)(bool)>(&FaceAction::process));
    _action.setText( getDisplayName());

    QString tooltip = getToolTip();
    if ( tooltip.isEmpty())
        tooltip = getDisplayName();
    _action.setToolTip( tooltip);
    if ( getWidget())
        getWidget()->setToolTip( tooltip);

    const QIcon* icon = getIcon();
    if ( icon)
        _action.setIcon(*icon);
    const QKeySequence* keys = getShortcut();
    if ( keys)
        _action.setShortcut(*keys);
    testSetEnabled();
    _init = true;
}   // end init


// protected
void FaceAction::setAsync( bool enable, QProgressUpdater::Ptr pupdater)
{
    _doasync = enable;
    _pupdater = pupdater;
    if ( !isAsync()) // Ignore the progress updater if not async
        _pupdater = nullptr;
}   // end setAsync


// protected
void FaceAction::progress( float propComplete) { if (_pupdater) _pupdater->processUpdate( propComplete);}

// protected
float FaceAction::progress() const { return _pupdater ? _pupdater->progress() : 1.0f;}


// protected
void FaceAction::setReady( FaceControl* fc, bool v)
{
    assert(fc);
    bool wasReady = _ready.has(fc);
    _ready.erase(fc);
    if ( v && testReady(fc))
        _ready.insert(fc);

    testSetEnabled();
    setChecked( testChecked(fc));

    if ( wasReady != _ready.has(fc))
        tellReady(fc, v);
}   // end setReady


// public
void FaceAction::addProcessOn( const ChangeEvent& r) { _revents.insert(r);}
void FaceAction::addPurgeOn( const ChangeEvent& p) { _pevents.insert(p);}
bool FaceAction::operator()( bool cs){ return process( cs);}


// public
bool FaceAction::execAfter( FaceAction* fa)
{
    assert(fa);
    // Check that the action being added is not already present (or this action).
    std::unordered_set<FaceAction*> allacts = _aacts;
    allacts.insert( _sacts.begin(), _sacts.end());
    allacts.insert( this);
    if ( !checkAfterActions( allacts, fa))
        return false;

    if ( fa->isAsync())
        _aacts.insert(fa);
    else
        _sacts.push_back(fa);
    return true;
}   // end execAfter


// private
bool FaceAction::checkAfterActions( std::unordered_set<FaceAction*>& acts, FaceAction* fa) const
{
    // If action already in given set, sequence of actions to be executed
    // has cycles which is not allowed.
    if ( acts.count(fa) > 0)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::checkAfterActions: Adding duplicate action instance to "
                  << debugActionName() << std::endl;
        return false;
    }   // end if

    bool rv = true;
    for ( FaceAction* a : _sacts) // Consolidate sequence actions from this action.
    {
        acts.insert(a);
        rv = a->checkAfterActions( acts, fa);
        if ( !rv)
            break;
    }   // end for
    return rv;
}   // end checkAfterActions


// public slot
bool FaceAction::process( bool checked)
{
    if ( !testEnabled())
        return false;

    _pmutex.lock();
    emit reportStarting();
    setChecked(checked);    // For external calls to ensure the action is checked

    _wset = _ready; // Copy out the ready set as is now into the work set
    bool enteredDoAction = false;
    if ( !doBeforeAction( _wset))  // Always in the GUI thread
    {
        _pmutex.unlock();
        ChangeEventSet cset;
        _wset.clear();
        emit reportFinished( cset, _wset, false);
    }   // end if
    else
    {
        const bool async = isAsync();
        if ( displayDebugStatusProgression())
            std::cerr << "--- STARTING " << debugActionName() << " ---" << (async ? " (ASYNC)" : "") << std::endl;

        enteredDoAction = true;

        if ( async)
        {
            progress(0.0f);
            FaceActionWorker *worker = new FaceActionWorker( this, &_wset);
            connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::doOnActionFinished);
            connect( worker, &FaceActionWorker::finished, worker, &QObject::deleteLater);
            worker->start();   // Asynchronous start
        }   // end else
        else
        {
            const bool rval = doAction( _wset);  // Blocks
            doOnActionFinished( rval);
        }   // end else
    }   // end else

    return enteredDoAction;
}   // end process


// public
void FaceAction::resetReady( const FaceControlSet& cset)
{
    clearReady();
    std::for_each( std::begin(cset), std::end(cset), [=](auto fc){ this->setReady(fc,true);});
    if ( cset.empty())
        testSetEnabled();
}   // end resetReady


// public
void FaceAction::resetReady( FaceControl* fc)
{
    clearReady();
    setReady(fc, true);
}   // end resetReady


// public
void FaceAction::clearReady()
{
    FaceControlSet oset = _ready; // Reset the ready set to that given
    std::for_each( std::begin(oset), std::end(oset), [=](auto fc){ this->setReady(fc,false);});
    testSetEnabled();
}   // end clearReady


// public
bool FaceAction::process( const FaceControlSet& cset, bool cflag)
{ 
    resetReady( cset);
    return process( cflag);
}   // end process


// public
bool FaceAction::process( FaceControl* fc, bool cflag)
{
    FaceControlSet fcs;
    fcs.insert(fc);
    return process( fcs, cflag);
}   // end process


// private
// Recursively set the change event set on the action sequence rooted on this action.
void FaceAction::setChangeEventSet( ChangeEventSet* cset)
{
    _cset = cset;
    for ( FaceAction* a : _sacts)
    {
        assert( a != this);
        a->setChangeEventSet( cset);
    }   // end for
}   // end setChangeEventSet


// private slot
void FaceAction::doOnActionFinished( bool rval)
{
    const std::string dname = debugActionName();

    ChangeEventSet* childChangeEvents = _cset;
    ChangeEventSet* parentChangeEvents = nullptr;
    if ( !childChangeEvents)
        childChangeEvents = parentChangeEvents = new ChangeEventSet;

    doAfterAction( *childChangeEvents, _wset, rval);

    progress(1.0f);
    if ( displayDebugStatusProgression())
        std::cerr << "--- FINISHED " << dname << " ---" << (isAsync() ? " (ASYNC)" : "") << std::endl;

    // Start asynchronous actions. Since these will end at undefined times (notifying others of their
    // completion via FaceActionManager), their change events are not consolidated into this action's.
    for ( FaceAction* a : _aacts)
        a->process(_wset); // ready set copied out

    // Queued actions need their change events consolidating so multiple of the same event aren't reported.
    // Only the "parent" action reports finished with the change event set.
    if ( parentChangeEvents)
        setChangeEventSet( parentChangeEvents); // Sets the change event set over

    for ( FaceAction* a : _sacts)
        a->process(_wset); // ready set copied out

    _pmutex.unlock();

    if ( parentChangeEvents)
    {
        emit reportFinished( *parentChangeEvents, _wset, rval);
        delete parentChangeEvents;
    }   // end if

    setChangeEventSet(nullptr);
    assert(_cset == nullptr);
    _wset.clear();  // Clear the working set
}   // end doOnActionFinished

