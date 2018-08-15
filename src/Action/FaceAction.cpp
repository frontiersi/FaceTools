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
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname)
    : _dname(dname), _icon(), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico)
    : _dname(dname), _icon(ico), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence& ks)
    : _dname(dname), _icon(ico), _keys(ks),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence& ks)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon& ico, const QKeySequence* ks)
    : _dname(dname), _icon(ico), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }

// public
FaceAction::FaceAction( const QString& dname, const QIcon* ico, const QKeySequence* ks)
    : _dname(dname), _icon(ico ? *ico : QIcon()), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _action(this), _doasync(false), _cset(nullptr), _pupdater(nullptr) { }


FaceAction::~FaceAction()
{
}   // end dtor


// public
QString FaceAction::getDisplayName() const { return _dname.isEmpty() ? QTools::PluginInterface::getDisplayName() : _dname; }
const QIcon* FaceAction::getIcon() const { return _icon.isNull() ? nullptr : &_icon;}
const QKeySequence* FaceAction::getShortcut() const { return _keys.isEmpty() ? nullptr : &_keys;}


// public
bool FaceAction::testSetEnabled( const QPoint* p)
{
//    std::cerr << "FaceTools::FaceAction::testSetEnabled: " << debugActionName() << std::endl;
    const bool enabled = testEnabled(p);
    if ( enabled && p)
        _testPoint = *p;
    else
        _testPoint = QPoint(-1,-1);
    setEnabled( enabled);
    if ( !enabled && isCheckable())
        setChecked(false);
    return isEnabled();
}   // end testSetEnabled


// public
void FaceAction::setEnabled( bool v)
{
    _action.setEnabled(v);
    if ( _action.isEnabled() != v)
    {
        std::cerr << "[ERROR] FaceTools::FaceAction::setEnabled: Inconsistent enabling of action for "
                  << debugActionName() << " setting to " << std::boolalpha << v << std::endl;
    }   // end if
    assert( _action.isEnabled() == v);
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

    _init = true;
    testSetEnabled();
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
    const bool wasReady = isReady(fc);
    _ready.erase(fc);
    if ( v && fc && testReady(fc))
        _ready.insert( fc);

    testSetEnabled();
    setChecked( testChecked(fc));

    if ( wasReady != isReady(fc))
    {
        assert(fc);
        tellReady( fc, v);
    }   // end if
}   // end setReady


// protected
void FaceAction::setReady( const FaceControlSet& fcs, bool v)
{
    std::for_each( std::begin(fcs), std::end(fcs), [=](auto fc){ this->setReady(fc, v);});
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
    if ( !isEnabled())
        return false;

    _pmutex.lock();
    emit reportStarting();
    setChecked(checked);    // For external calls to ensure the action is checked

    _wset = _ready; // Copy in the ready set (may be empty)
    bool enteredDoAction = false;
    QPoint mpoint = _testPoint;

    if ( !doBeforeAction( _wset, mpoint))  // Always in the GUI thread
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
            FaceActionWorker *worker = new FaceActionWorker( this, &_wset, mpoint);
            connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::doOnActionFinished);
            connect( worker, &FaceActionWorker::finished, worker, &QObject::deleteLater);
            worker->start();   // Asynchronous start
        }   // end else
        else
        {
            const bool rval = doAction( _wset, mpoint);  // Blocks
            doOnActionFinished( rval);
        }   // end else
    }   // end else

    return enteredDoAction;
}   // end process


// public
bool FaceAction::process( FaceControl* fc, bool cflag)
{
    _ready.clear();
    if ( fc)
        setReady( fc, true);
    return process( cflag);
}   // end process


// public
bool FaceAction::process( const FaceControlSet& fcs, bool cflag)
{
    _ready.clear();
    setReady( fcs, true);
    return process( cflag);
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
        a->process(_wset);

    // Queued actions need their change events consolidating so multiple of the same event aren't reported.
    // Only the "parent" action reports finished with the change event set.
    if ( parentChangeEvents)
        setChangeEventSet( parentChangeEvents); // Sets the change event set over

    for ( FaceAction* a : _sacts)
        a->process(_wset);

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

