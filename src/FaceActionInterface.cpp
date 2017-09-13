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

#include <FaceActionInterface.h>
#include <FaceActionWorker.h>
using FaceTools::FaceActionGroup;
using FaceTools::FaceAction;


// public
FaceActionGroup::FaceActionGroup() : _group(this), _menu(NULL), _toolbar(NULL) {}   // end ctor


// public
FaceActionGroup::~FaceActionGroup()
{
    foreach ( const FaceAction* faction, _actionList)
        delete faction;
    if ( _menu)
        delete _menu;
    _menu = NULL;
    if (_toolbar)
        delete _toolbar;
}   // end dtor


// protected
bool FaceActionGroup::addAction( FaceAction* faction)
{
    const std::string nm = faction->getDisplayName().toStdString();
    if ( _actions.count(nm) > 0)
        return false;

    _actions[nm] = faction;
    _actionList << faction;
    return true;
}   // end addAction


namespace {
void addToWidget( const QList<FaceAction*>& actions, QWidget* widget)
{
    foreach ( FaceAction* fa, actions)
        widget->addAction( fa->qaction());
}   // end addToWidget
}   // end namespace


const QMenu* FaceActionGroup::createMenu()
{
    if ( !_menu)
    {
        _menu = new QMenu( getDisplayName(), NULL);
        addToWidget( _actionList, _menu);
    }   // end if
    return _menu;
}   // end createMenu


const QToolBar* FaceActionGroup::createToolBar()
{
    if ( !_toolbar)
    {
        _toolbar = new QToolBar( getDisplayName(), NULL);
        addToWidget( _actionList, _toolbar);
    }   // end if
    return _toolbar;
}   // end createToolBar


void FaceActionGroup::addTo( QMenu* menu) const
{
    if ( !menu->actions().empty())
        menu->addSeparator();
    addToWidget( _actionList, menu);
}   // end addTo


void FaceActionGroup::addTo( QToolBar* toolbar) const
{
    if ( !toolbar->actions().empty())
        toolbar->addSeparator();
    addToWidget( _actionList, toolbar);
}   // end addTo


// public virtual
QStringList FaceActionGroup::getInterfaceIds() const
{
    QStringList qlist;
    foreach ( const FaceAction* fa, _actionList)
        qlist << fa->getDisplayName();
    return qlist;
}   // end getInterfaceIds


// public virtual
FaceAction* FaceActionGroup::getInterface( const QString& iname) const
{
    const std::string nm = iname.toStdString();
    if ( _actions.count(nm) == 0)
        return NULL;
    return _actions.at(nm);
}   // end getInterface





// public
FaceAction::FaceAction() : FaceTools::FaceActionInterface(),
    _action(this), _doasync(false), _pupdater(NULL), _fmaw(NULL)
{
    connect( &_action, &QAction::triggered, this, &FaceAction::process);
}   // end ctor


// protected
void FaceAction::init()
{
    _action.setText( this->getDisplayName());
    const QIcon* icon = this->getIcon();
    if ( icon)
        _action.setIcon(*icon);
    const QKeySequence* keys = this->getShortcut();
    if ( keys)
        _action.setShortcut(*keys);
}   // end init


// public
void FaceAction::setAsync( bool enable, QTools::QProgressUpdater* pupdater)
{
    _doasync = enable;
    _pupdater = pupdater;
    if ( !_doasync) // Ignore the progress updater if not async
        _pupdater = NULL;
}   // end setAsync


// public
void FaceAction::connectToolButton( QToolButton* tb)
{
    tb->addAction( &_action);
    tb->setIcon( _action.icon());
    tb->setIconSize( QSize(20,20));
    tb->setMinimumSize( QSize(26,26));
    tb->setStyleSheet( "QToolButton::menu-indicator { image: none; }"); // Remove the menu-indicator
    tb->connect( tb, &QToolButton::clicked, &_action, &QAction::triggered);
}   // end connectToolButton


// public
void FaceAction::connectButton( QPushButton* b)
{
    b->addAction( &_action);
    b->setIcon( _action.icon());
    b->setMinimumSize( QSize(26,26));
    b->connect( b, &QPushButton::clicked, &_action, &QAction::triggered);
}   // end connectButton


// protected
void FaceAction::progress( float propComplete)
{
    if (_pupdater)
        _pupdater->processUpdate( propComplete);
}   // end progress



// private slot
bool FaceAction::process()
{
    if ( !_doasync)
    {
        bool rval = doAction();  // Blocks
        emit finished(rval);
        return rval;
    }   // end if

    _fmaw = new FaceTools::FaceActionWorker( this);
    connect( _fmaw, &FaceTools::FaceActionWorker::workerFinished, this, &FaceAction::finished);
    connect( _fmaw, SIGNAL( finished()), _fmaw, SLOT(deleteLater()));
    _fmaw->start();   // Asynchronous; immediately return and listen for finished(bool)
    return true;
}   // end process
