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
    _doasync = false;
    _isWorking = false;
    _worker = nullptr;
    _unlocked = true;
    _pevents = _tevents = _revents = Event::NONE;
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


void FaceAction::setChecked( bool b)
{
    _action.setChecked(b);
}   // end setChecked


void FaceAction::refresh( Event e)
{
    const bool chk = update( e);
    if ( _action.isCheckable() && _action.isChecked() != chk)
    {
        const QSignalBlocker blocker( _action);
        _action.setChecked( chk);
    }   // end if

    _action.setEnabled( _unlocked && isAllowed( e));
    if ( widget())
        widget()->setEnabled( isEnabled());
}   // end refresh


void FaceAction::setLocked( bool v)
{
    _unlocked = !v;
    refresh( Event::USER);
}   // end setLocked


bool FaceAction::primeMousePos( const QPoint &p)
{
    if ( p != _mpos)
    {
        _mpos = p;
        refresh( Event::USER);
        if ( !isEnabled())
            _mpos = QPoint(-1,-1);
    }   // end if
    return isEnabled();
}   // end primeMousePos


void FaceAction::addPurgeEvent( Event e) { _pevents |= e;}
void FaceAction::addTriggerEvent( Event e) { _tevents |= e;}
void FaceAction::addRefreshEvent( Event e) { _revents |= e;}
bool FaceAction::isPurgeEvent( Event e) const { return (_pevents & e) != Event::NONE;}
bool FaceAction::isTriggerEvent( Event e) const { return (_tevents & e) != Event::NONE;}
bool FaceAction::isRefreshEvent( Event e) const { return (_revents & e) != Event::NONE;}


// protected
void FaceAction::setAsync( bool async) { _doasync = async;}


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
    const QKeySequence* ks = shortcut();
    if ( ks)
        _action.setShortcut(*ks);

    if ( parent)
        setParent(parent);

    _action.setVisible(true);
    postInit();
}   // end _init


bool FaceAction::execute( Event e)
{
    if ( !isAllowed(e))
        return false;

    _action.setEnabled(false);
    bool enteredDoAction = false;

    if ( !doBeforeAction(e))  // Always in the GUI thread
    {
#ifndef NDEBUG
        std::cerr << "Cancelled: " << debugName() << std::endl;
#endif
        refresh( e);
        emit onEvent( Event::CANCEL);
    }   // end if
    else
    {
        _isWorking = true;
        enteredDoAction = true;

#ifndef NDEBUG
        std::cerr << "  Started: " << debugName() << "  by event(s) " << e;
#endif
        if ( isAsync())
        {
#ifndef NDEBUG
            std::cerr << " in new thread " << std::endl;
#endif
            if ( e == Event::USER)
                MS::setLockSelected(true);
            _worker = new FaceActionWorker( this, e);
            connect( _worker, &FaceActionWorker::onWorkFinished, this, &FaceAction::_endExecute);
            _worker->start();   // Asynchronous start
        }   // end else
        else
        {
#ifndef NDEBUG
            std::cerr << std::endl;
#endif
            doAction(e);  // Blocks
            _endExecute(e);
        }   // end else
    }   // end else

    return enteredDoAction;
}   // end execute


Event FaceAction::doAfterAction( Event e)
{
    if ( e == Event::USER && _action.isCheckable() && !isAsync())
        MS::showStatus( displayName() + (isChecked() ? " ON" : " OFF"), 5000);
    return Event::NONE;
}   // end doAfterAction


void FaceAction::endNow()
{
    std::cerr << "ENDING NOW" << std::endl;
}   // end endNow


// private slot
void FaceAction::_endExecute( Event e)   // Always in GUI thread
{
    _isWorking = false;
    if ( _worker)
    {
        delete _worker;
        _worker = nullptr;
    }   // end if

    MS::setLockSelected(false);
    Event fev = doAfterAction( e);
    _mpos = QPoint(-1,-1);
#ifndef NDEBUG
    std::cerr << " Finished: " << debugName() << " did event(s) " << fev << std::endl;
#endif
    refresh( e);
    emit onEvent( fev);
}   // end _endExecute


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
