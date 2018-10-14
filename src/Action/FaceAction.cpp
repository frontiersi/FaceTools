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

#include <FaceAction.h>
#include <FaceActionWorker.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::FaceActionWorker;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::Action::EventId;
using FaceTools::Action::EventSet;
using FaceTools::Action::EPR;
using FaceTools::Action::ResponsePredicate;
using FaceTools::Action::ProcessFlagPredicate;
using QTools::QProgressUpdater;


// public
FaceAction::FaceAction()
    : _dname(""), _icon(),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn)
    : _dname(dn), _icon(), _keys(),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon& ico)
    : _dname(dn), _icon(ico), _keys(),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon* ico)
    : _dname(dn), _icon(ico ? *ico : QIcon()), _keys(),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : _dname(dn), _icon(ico), _keys(ks),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon* ico, const QKeySequence& ks)
    : _dname(dn), _icon(ico ? *ico : QIcon()), _keys(ks),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon& ico, const QKeySequence* ks)
    : _dname(dn), _icon(ico), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }

// public
FaceAction::FaceAction( const QString& dn, const QIcon* ico, const QKeySequence* ks)
    : _dname(dn), _icon(ico ? *ico : QIcon()), _keys(ks ? *ks : QKeySequence()),
      _init(false), _visible(true), _defaultCheckState(false), _allowOnToolbar(true),
      _action(this), _doasync(false), _pupdater(nullptr), _dialog(nullptr) { }


// public
QString FaceAction::getDisplayName() const { return _dname.isEmpty() ? QTools::PluginInterface::getDisplayName() : _dname; }
const QIcon* FaceAction::getIcon() const { return _icon.isNull() ? nullptr : &_icon;}
const QKeySequence* FaceAction::getShortcut() const { return _keys.isEmpty() ? nullptr : &_keys;}


// public
void FaceAction::setCheckable( bool b, bool ival)
{
    _action.setCheckable(b);
    _action.setChecked(ival);
    _defaultCheckState = ival;
}   // end setCheckable


// public
void FaceAction::setAsDialogShower( QDialog* d)
{
    setCheckable(false,false);
    _dialog = d;
}   // end setAsDialogShower


// public
bool FaceAction::testSetEnabled( const QPoint* p)
{
    const bool enabled = testEnabled(p);
    if ( enabled && p)
        _testPoint = *p;
    else
        _testPoint = QPoint(-1,-1);
    setEnabled( enabled);
    return isEnabled();
}   // end testSetEnabled


// public
void FaceAction::setEnabled( bool v)
{
    _action.setEnabled(v);
    if ( _action.isEnabled() != v)
    {
        std::cerr << "[ERROR] FaceTools::FaceAction::setEnabled: Inconsistent enabling of action for "
                  << dname() << " setting to " << std::boolalpha << v << std::endl;
    }   // end if
    assert( _action.isEnabled() == v);
    if ( getWidget())
        getWidget()->setEnabled(v);
}   // end setEnabled


// protected
void FaceAction::init() // Called by FaceActionManager after constructor finished
{
    if ( _init)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::init: Cannot initialise more than once!" << std::endl;
        return;
    }   // end if

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
    postInit();
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


void FaceAction::setReady( FV* fv, bool v)
{
    const bool wasReady = isReady(fv);
    _ready.erase(fv);
    if ( v && fv && testReady(fv))
        _ready.insert( fv);

    testSetEnabled();
    //setChecked( fv ? testIfCheck(fv) : _defaultCheckState);
    setChecked( testIfCheck(fv));

    if ( wasReady != isReady(fv))
    {
        assert(fv);
        tellReady( fv, v);
    }   // end if
}   // end setReady


void FaceAction::setReady( const FVS& fvs, bool v)
{
    std::for_each( std::begin(fvs), std::end(fvs), [v,this](FV* fv){ this->setReady(fv, v);});
    testSetEnabled();
}   // end setReady


void FaceAction::clearReady()
{
    FVS rset = ready();
    for ( FV* fv : rset)
        setReady( fv, false);
    testSetEnabled();
}   // end clearReady


void FaceAction::resetReady( FV* fv)
{
    clearReady();
    setReady(fv, true);
}   // end resetReady


void FaceAction::setPurgeOnEvent( EventId e) { _pevents.insert(e);}


// private
void FaceAction::checkOverwritingResponseEvent( EventId e) const
{
    if ( _eprs.count(e) > 0)
    {
        std::cerr << "[WARNING] FaceTools::FaceAction::setRespondToEvent(If): Overwritting event on action "
                  << dname() << " <" << this << ">" << std::endl;
    }   // end if
}   // end checkOverwritingResponseEvent


// public
void FaceAction::setRespondToEvent( EventId e, bool cval)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, cval);
}   // end setRespondToEvent


// public
void FaceAction::setRespondToEvent( EventId e, const ProcessFlagPredicate& fp)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, fp);
}   // end setRespondToEvent


// public
void FaceAction::setRespondToEventIf( EventId e, const ResponsePredicate& rp, bool cval)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, rp, cval);
}   // end setRespondToEventIf


// public
void FaceAction::setRespondToEventIf( EventId e, const ResponsePredicate& rp, const ProcessFlagPredicate& fp)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, rp, fp);
}   // end setRespondToEventIf


// public
void FaceAction::setRespondToEventIfAllReady( EventId e, bool cval)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, [this](FVS& fvs)
            { return std::all_of(std::begin(fvs), std::end(fvs), [this](FV* fv){ return this->testReady(fv);});}, cval);
}   // end setRespondToEventIfAllReady


// public
void FaceAction::setRespondToEventIfAllReady( EventId e, const ProcessFlagPredicate& fp)
{
    checkOverwritingResponseEvent( e);
    _eprs[e] = EPR( e, [this](FVS& fvs)
            { return std::all_of(std::begin(fvs), std::end(fvs), [this](FV* fv){ return this->testReady(fv);});}, fp);
}   // end setRespondToEventIfAllReady


bool FaceAction::operator()( bool cs){ return process( cs);}


// public slot
bool FaceAction::process( bool checked)
{
    if ( !isEnabled() && checked)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::process(true): on " << dname() << "; action is not enabled!" << std::endl;
        assert(false);
        return false;
    }   // end if

    // Is this action meant to simply show a dialog?
    if ( _dialog)
    {
        _dialog->show();
        _dialog->raise();
        _dialog->activateWindow();
        return true;
    }   // end if

    _pmutex.lock();
    setChecked(checked);    // For external calls to ensure the action is checked
    emit reportStarting();

    _wset = _ready; // Copy in the ready set (may be empty)
    bool enteredDoAction = false;
    QPoint mpoint = _testPoint;

    if ( !doBeforeAction( _wset, mpoint))  // Always in the GUI thread
    {
        _pmutex.unlock();
        EventSet cset;
        _wset.clear();
        emit reportFinished( cset, _wset, false);
    }   // end if
    else
    {
        const bool async = isAsync();
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
bool FaceAction::process( FV* fv, bool cflag)
{
    _ready.clear();
    if ( fv)
        setReady( fv, true);
    return process( cflag);
}   // end process


// public
bool FaceAction::process( const FVS& fvs, bool cflag)
{
    _ready.clear();
    setReady( fvs, true);
    return process( cflag);
}   // end process


// private slot
void FaceAction::doOnActionFinished( bool rval)
{
    EventSet cset;
    doAfterAction( cset, _wset, rval);
    progress(1.0f);
    _pmutex.unlock();
    emit reportFinished( cset, _wset, rval);
    _wset.clear();  // Clear the working set
}   // end doOnActionFinished
