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

#include <Action/FaceAction.h>
#include <Action/FaceActionWorker.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <QSignalBlocker>
#include <QThread>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceActionWorker;
using FaceTools::Action::FaceAction;
using FaceTools::Action::UndoState;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


FaceAction::FaceAction()
    : _action(this), _dname("") { _pinit();}

FaceAction::FaceAction( const QString& dn)
    : _action(this), _dname(dn) { _pinit();}

FaceAction::FaceAction( const QString& dn, const QIcon& ico)
    : _action(this), _dname(dn), _icon(ico) { _pinit();}

FaceAction::FaceAction( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : _action(this), _dname(dn), _icon(ico), _keys(ks) { _pinit();}


void FaceAction::_pinit()
{
    _doasync = _reentrant = false;
    _runCount = 0;
    _unlocked = true;
    _pevents = _tevents = Event::NONE;
    if ( _dname.isEmpty())
        _dname = QTools::PluginInterface::displayName();
    _debugName = "\"" + _dname.replace( "\n", " ").remove('&').toStdString() + "\"";
    _mpos = QPoint(-1,-1);
}   // end _pinit


const std::string& FaceAction::debugName() const { return _debugName;}


void FaceAction::setDisplayName( const QString& dname)
{
    _dname = dname;
    _action.setText( displayName());
}   // end setDisplayName


void FaceAction::setToolTip( const QString& ttip) { _action.setToolTip( ttip);}


QString FaceAction::displayName() const { return _dname;}
const QIcon* FaceAction::icon() const { return _icon.isNull() ? nullptr : &_icon;}
const QKeySequence* FaceAction::shortcut() const { return _keys.isEmpty() ? nullptr : &_keys;}


void FaceAction::setCheckable( bool b, bool ival)
{
    _action.setCheckable(b);
    _action.setChecked(ival);
}   // end setCheckable


void FaceAction::refreshState( Event e)
{
    e |= Event::USER;
    const bool chk = checkState( e);
    if ( _action.isCheckable() && _action.isChecked() != chk)
    {
        const QSignalBlocker blocker( _action);
        _action.setChecked( chk);
    }   // end if

    _action.setEnabled( _unlocked && isAllowed( e));
    if ( widget())
        widget()->setEnabled( isEnabled());
}   // end refreshState


void FaceAction::setLocked( bool v)
{
    _unlocked = !v;
    refreshState();
}   // end setLocked


void FaceAction::primeMousePos(const QPoint &p) { _mpos = p;}


void FaceAction::addPurgeEvent( Event e) { _pevents |= e;}
void FaceAction::addTriggerEvent( Event e) { _tevents |= e;}
bool FaceAction::isPurgeEvent( Event e) const { return (_pevents & e) != Event::NONE;}
bool FaceAction::isTriggerEvent( Event e) const { return (_tevents & e) != Event::NONE;}


// protected
void FaceAction::setAsync( bool async, bool reentrant)
{
    _doasync = async;
    _reentrant = async && reentrant;
}   // end setAsync


void FaceAction::_init( QWidget* parent) // Called by FaceActionManager after constructor finished
{
    if ( _action.isCheckable())
        connect( &_action, &QAction::toggled, [this](){ execute(Event::USER);});
    else
        connect( &_action, &QAction::triggered, [this](){ execute(Event::USER);});

    _action.setText( displayName());

    QString tooltip = toolTip();
    if ( !tooltip.isEmpty())
        _action.setToolTip( tooltip);

    QString whatsthis = whatsThis();
    if ( whatsthis.isEmpty())
        whatsthis = tooltip;
    if ( !whatsthis.isEmpty())
        _action.setWhatsThis( whatsthis);

    if ( widget())
    {
#ifndef NDEBUG
        std::cerr << debugName() << " initialising widget" << std::endl;
#endif
        widget()->setToolTip( tooltip);
    }   // end if

    const QIcon* ic = icon();
    if ( ic)
        _action.setIcon(*ic);
    const QKeySequence* keys = shortcut();
    if ( keys)
        _action.setShortcut(*keys);

    if ( parent)
        setParent(parent);

    _action.setVisible(true);
    postInit();
}   // end _init


bool FaceAction::execute( Event e)
{
#ifndef NDEBUG
    if ( e != Event::USER && !isTriggerEvent(e))
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::execute: Starting from non-user or non-trigger event!" << std::endl;
    assert( e == Event::USER || isTriggerEvent(e));
#endif

    if ( !isAllowed(e))
        return false;

    _runCount++;
#ifndef NDEBUG
    assert( _runCount <= 1);
#endif
    _action.setEnabled(false);
    bool enteredDoAction = false;

    if ( !doBeforeAction(e))  // Always in the GUI thread
    {
#ifndef NDEBUG
        std::cerr << "Cancelled: " << debugName() << std::endl;
#endif
        _runCount--;
        refreshState();
        emit onEvent( Event::CANCELLED);
    }   // end if
    else
    {
#ifndef NDEBUG
        std::cerr << "  Started: " << debugName() << "  by event(s) " << e;
#endif
        enteredDoAction = true;
        if ( isAsync())
        {
#ifndef NDEBUG
            std::cerr << " in new thread " << std::endl;
#endif
            if ( e == Event::USER)
                MS::setLockSelected(true);
            FaceActionWorker *worker = new FaceActionWorker( this, e);
            connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::endExecute);
            connect( worker, &FaceActionWorker::finished, worker, &FaceActionWorker::deleteLater);
            worker->start();   // Asynchronous start
        }   // end else
        else
        {
#ifndef NDEBUG
            std::cerr << std::endl;
#endif
            doAction(e);  // Blocks
            endExecute(e);
        }   // end else
    }   // end else

    return enteredDoAction;
}   // end execute


Event FaceAction::doAfterAction( Event e)
{
    Event egrp = Event::NONE;
    // Don't display anything in the status bar for actions that weren't triggered explicitly by the user.
    if ( e == Event::USER)
    {
        QString dname = displayName();
        if ( _action.isCheckable())
            dname += (isChecked() ? " ON" : " OFF");
        int msecs = 2000;
        if ( isAsync())
        {
            dname = "Finished " + displayName();
            msecs = 5000;
        }   // end if
        MS::showStatus( dname, msecs);
    }   // end if
    return egrp;
}   // end doAfterAction


// private slot
void FaceAction::endExecute( Event e)   // Always in GUI thread
{
    MS::setLockSelected(false);
    Event fev = doAfterAction( e);
    _mpos = QPoint(-1,-1);
#ifndef NDEBUG
    std::cerr << " Finished: " << debugName() << " did event(s) " << fev << std::endl;
#endif
    _runCount--;
    refreshState();
    emit onEvent( fev);
}   // end endExecute


void FaceAction::saveState( UndoState&) const
{
    std::cerr << "[ERROR] FaceTools::Action::FaceAction::saveState: [" << debugName() << "] "
              << "This function must be overridden to set the UndoState!" << std::endl;
    assert(false);
}   // end saveState


void FaceAction::restoreState( const UndoState&)
{
    std::cerr << "[ERROR] FaceTools::Action::FaceAction::restoreState: [" << debugName() << "] "
              << "This function must be overridden to restore using the UndoState!" << std::endl;
    assert(false);
}   // end restoreState
