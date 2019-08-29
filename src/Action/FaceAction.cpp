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

#include <FaceAction.h>
#include <FaceActionWorker.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <QThread>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceActionWorker;
using FaceTools::Action::FaceAction;
using FaceTools::Action::UndoState;
using FaceTools::Action::EventGroup;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


FaceAction::FaceAction()
    : _action(this), _dname("") { pinit();}

FaceAction::FaceAction( const QString& dn)
    : _action(this), _dname(dn) { pinit();}

FaceAction::FaceAction( const QString& dn, const QIcon& ico)
    : _action(this), _dname(dn), _icon(ico) { pinit();}

FaceAction::FaceAction( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : _action(this), _dname(dn), _icon(ico), _keys(ks) { pinit();}


// private
void FaceAction::pinit()
{
    _init = _doasync = _reentrant = false;
    _runCount = 0;
    _unlocked = true;
    _pevents = _tevents = Event::NONE;
    if ( _dname.isEmpty())
        _dname = QTools::PluginInterface::displayName();
    _debugName = "\"" + _dname.replace( "\n", " ").remove('&').toStdString() + "\"";
    _mpos = QPoint(-1,-1);
}   // end pinit


const std::string& FaceAction::debugName() const { return _debugName;}


void FaceAction::setDisplayName( const QString& dname)
{
    _dname = dname;
    _action.setText( displayName());
}   // end setDisplayName


void FaceAction::setToolTip( const QString& ttip)
{
    _action.setToolTip( ttip);
}   // end setToolTip


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
    const bool chk = checkState( e);
    if ( _action.isCheckable())
        setChecked( chk);
    _action.setEnabled( _unlocked && checkEnable( e));
    if ( widget())
        widget()->setEnabled( isEnabled());
}   // end updateState


void FaceAction::setLocked( bool v)
{
    _unlocked = !v;
    refreshState();
}   // end setLocked


void FaceAction::primeMousePos(const QPoint &p) { _mpos = p;}


void FaceAction::addPurgeEvent( EventGroup e) { _pevents = EventGroup(_pevents).add(e);}
void FaceAction::addTriggerEvent( EventGroup e) { _tevents = EventGroup(_tevents).add(e);}
bool FaceAction::isPurgeEvent( EventGroup e) const { return EventGroup(Event(_pevents)).has(e);}
bool FaceAction::isTriggerEvent( EventGroup e) const { return EventGroup(Event(_tevents)).has(e);}


// protected
void FaceAction::setAsync( bool async, bool reentrant)
{
    _doasync = async;
    _reentrant = async && reentrant;
}   // end setAsync


void FaceAction::init( QWidget* parent) // Called by FaceActionManager after constructor finished
{
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
    _init = true;
    postInit();
}   // end init


bool FaceAction::execute( Event e)
{
#ifndef NDEBUG
    if ( !isEnabled())
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::process(): " << debugName() << " action is not enabled!" << std::endl;
        assert(false);
    }   // end if
#endif

    assert( e == Event::USER || isTriggerEvent(e));

    _runCount++;
    _action.setEnabled(false);
    bool enteredDoAction = false;

#ifndef NDEBUG
    std::cerr << " => " << debugName() << " (" << EventGroup(e).name() << ") thread ID = " << QThread::currentThreadId() << std::endl;
#endif
    if ( !doBeforeAction(e))  // Always in the GUI thread
    {
#ifndef NDEBUG
        std::cerr << "  ! " << debugName() << std::endl;
#endif
        _runCount--;
        refreshState();
        emit onEvent( Event::ACT_CANCELLED);
    }   // end if
    else
    {
        enteredDoAction = true;
        if ( isAsync())
        {
            if ( EventGroup(e).is(Event::USER))
                MS::setCursor( Qt::CursorShape::BusyCursor);

            FaceActionWorker *worker = new FaceActionWorker( this, e);
            connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::endExecute);
            connect( worker, &FaceActionWorker::finished, worker, &FaceActionWorker::deleteLater);
            worker->start();   // Asynchronous start
        }   // end else
        else
        {
            doAction(e);  // Blocks
            endExecute(e);
        }   // end else
    }   // end else

    return enteredDoAction;
}   // end execute


void FaceAction::doAfterAction( Event e)
{
    // Don't display anything in the status bar for actions that weren't triggered explicitly by the user.
    if ( !EventGroup(e).is(Event::USER))
        return;

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
}   // end doAfterAction


// private slot
void FaceAction::endExecute( Event e)   // Always in GUI thread
{
    if ( isAsync() && EventGroup(e).is(Event::USER))
        MS::restoreCursor();

    doAfterAction( e);
    _mpos = QPoint(-1,-1);
#ifndef NDEBUG
    std::cerr << " <= " << debugName() << " thread ID = " << QThread::currentThreadId() << std::endl;
#endif
    _runCount--;
    refreshState();
    emit onEvent( Event::ACT_COMPLETE);
}   // end endExecute


UndoState::Ptr FaceAction::makeUndoState() const
{
    std::cerr << "[ERROR] FaceTools::Action::FaceAction::makeUndoState: [" << debugName() << "] "
              << "This function must be overridden to provide a custom UndoState!" << std::endl;
    assert(false);
    return nullptr;
}   // end makeUndoState


void FaceAction::restoreState( const UndoState*)
{
    std::cerr << "[ERROR] FaceTools::Action::FaceAction::restoreState: [" << debugName() << "] "
              << "This function must be overridden to provide custom UndoState restore!" << std::endl;
    assert(false);
}   // end restoreState
