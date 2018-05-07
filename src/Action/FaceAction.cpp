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
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceAction;
using FaceTools::Action::FaceActionInterface;
using FaceTools::Action::FaceActionWorker;
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;


// public
FaceAction::FaceAction( bool dbo)
    : FaceActionInterface(),
    _init(false), _disableBeforeOther(dbo),
    _action(this), _doasync(false), _pupdater(NULL)
{
}   // end ctor


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
    connect( &_action, &QAction::triggered, this, &FaceAction::process);
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
void FaceAction::setAsync( bool enable, QTools::QProgressUpdater* pupdater)
{
    _doasync = enable;
    _pupdater = pupdater;
    if ( !_doasync) // Ignore the progress updater if not async
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
    const bool wascontrolled = _controlled.has(fc);
    const bool wasready = _ready.has(fc);
    _controlled.erase(fc);
    _ready.erase(fc);
    v = v && fc && fc->viewer();    // FaceControls without viewers can never be selected
    if ( fc && v)
    {
        _controlled.insert(fc);
        if ( testReady(fc))
            _ready.insert(fc);
    }   // end if

    setEnabled( testEnabled());
    if ( fc)
    {
        if ( wascontrolled != _controlled.has(fc))  // Only call on change to controlled set membership
            this->tellSelected( fc, v);
        if ( wasready != _ready.has(fc))    // Only call on change to ready set membership
            this->tellReady( fc, _ready.has(fc));
    }   // end if
}   // end setSelected


// protected
size_t FaceAction::recheckReadySet()
{
    FaceControlSet nset;
    for ( FaceControl* fc : _ready)
    {
        if ( testReady(fc))
            nset.insert(fc);
    }   // end for
    _ready = nset;
    setEnabled( testEnabled());
    return _ready.size();
}   // end recheckReadySet


// public
void FaceAction::addChangeTo( const ChangeEvent& c) { _cevents.insert(c);}
void FaceAction::addRespondTo( const ChangeEvent& r) { _revents.insert(r);}
bool FaceAction::operator()(){ return process();}


// protected
void FaceAction::respondToChange( FaceControl* fc)
{
    _ready.erase(fc);
    if ( fc && isSelected(fc) && testReady(fc))
        _ready.insert(fc);
    setEnabled( testEnabled());
}   // end respondToChange


// public slot
bool FaceAction::process( bool checked)
{
    if ( !_init)
    {
        std::cerr << "[ERROR] FaceTools::Action::FaceAction::process: FaceAction::init() not called!" << std::endl;
        assert(_init);
        return false;
    }   // end if

    setChecked(checked);
    const std::string dname = debugActionName();
    if ( displayDebugStatusProgression())
    {
        std::cerr << std::endl;
        std::cerr << "==={" << dname << "}=== STARTING" << std::endl;
    }   // end if

    setEnabled(false);
    emit reportStarting( &_ready);
    if ( !doBeforeAction( _ready))  // Always in the GUI thread
    {
        if ( displayDebugStatusProgression())
            std::cerr << "___{" << dname << "}___ CANCELLED" << std::endl;
        setEnabled( testEnabled());
        emit reportFinished( NULL);
        return false;   // Test for cancelled action
    }   // end if

    _pready = _ready;   // Cache the _ready set going in to the action - finished action will need to recheck this one.
    if ( !_doasync)
    {
        if ( displayDebugStatusProgression())
            std::cerr << "---{" << dname << "}--- WORKING" << std::endl;
        const bool rval = doAction( _ready);  // Blocks - doAction may change membership of _ready
        doOnActionFinished( rval);
    }   // end if
    else
    {
        if ( displayDebugStatusProgression())
            std::cerr << "---{" << dname << "}--- WORKING (BG)" << std::endl;
        FaceActionWorker *worker = new FaceActionWorker( this, &_ready);
        connect( worker, &FaceActionWorker::workerFinished, this, &FaceAction::doOnActionFinished);
        connect( worker, &FaceActionWorker::finished, worker, &QObject::deleteLater);
        worker->start();   // Asynchronous start
    }   // end else
    return true;
}   // end process


// public
void FaceAction::execAfter( FaceAction* fa)
{
    _eacts.push_back(fa);
}   // end execAfter


// private
void FaceAction::chain( const FaceControlSet& cset)
{
    _controlled.clear();
    _ready.clear();
    for ( FaceControl* fc : cset)
        setSelected( fc, true);
    process();  // Return value ignored
}   // end chain


// private slot
void FaceAction::doOnActionFinished( bool rval)
{
    doAfterAction( _ready, rval);   // Possible that _ready != _pready
    std::for_each( std::begin(_eacts), std::end(_eacts), [&](auto a){ a->chain(_ready);}); // Chain to execAfter actions
    progress(1.0f);
    FaceControlSet oldrdy = _ready; // Copy out for reportChanged to potential listening actions
    _ready = _pready;               // Reinstate the old _ready set to allow rechecking.
    _pready.clear();                // Pre-action ready no longer needed.
    recheckReadySet();              // Allows this action to enable/disable itself first
    for ( FaceControl* fc : oldrdy) // Allow "listening" FaceActions to update themselves on the affected FaceControls
        emit reportChanged( fc);
    if ( displayDebugStatusProgression())
    {
        const std::string dname = debugActionName();
        std::cerr << "___{" << dname << "}___ FINISHED" << std::endl;
    }   // end if
    emit reportFinished( &oldrdy);   // Used by FaceActionManager to call setEnabled on all actions registered with it.
}   // end doOnActionFinished


// protected
void FaceAction::doAfterAction( const FaceControlSet& fst, bool)
{
    std::unordered_set<FaceModelViewer*> vws;   // Collect viewers for calls to updateRender
    std::for_each( std::begin(fst), std::end(fst), [&]( auto f){ vws.insert(f->viewer());});
    std::for_each( std::begin(vws), std::end(vws), [&]( auto v){ v->updateRender();});
}   // end doAfterAction
