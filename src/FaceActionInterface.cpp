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
using FaceTools::FaceActionWorker;
using FaceTools::FaceActionGroup;
using FaceTools::FaceActionInterface;
using FaceTools::FaceAction;


// public
FaceActionGroup::FaceActionGroup() : _group(this), _menu(NULL), _toolbar(NULL) {}   // end ctor


// public
FaceActionGroup::~FaceActionGroup()
{
    typedef std::pair<std::string, FaceActionInterface*> FAPair;
    foreach ( const FAPair& fa, _actions)
        delete fa.second;
    _actions.clear();
    if ( _menu)
        delete _menu;
    _menu = NULL;
    if (_toolbar)
        delete _toolbar;
    _toolbar = NULL;
}   // end dtor


// protected
bool FaceActionGroup::addAction( FaceActionInterface* faction)
{
    const std::string nm = faction->getDisplayName().toStdString();
    if ( _actions.count(nm) > 0)
        return false;

    _actions[nm] = faction;
    return true;
}   // end addAction


namespace {
void addToWidget( const boost::unordered_map<std::string, FaceActionInterface*>& actions, QWidget* widget)
{
    typedef std::pair<std::string, FaceActionInterface*> FAPair;
    foreach ( const FAPair& fa, actions)
        widget->addAction( fa.second->qaction());
}   // end addToWidget
}   // end namespace


const QMenu* FaceActionGroup::createMenu()
{
    if ( !_menu)
    {
        _menu = new QMenu( getDisplayName(), NULL);
        addToWidget( _actions, _menu);
    }   // end if
    return _menu;
}   // end createMenu


const QToolBar* FaceActionGroup::createToolBar()
{
    if ( !_toolbar)
    {
        _toolbar = new QToolBar( getDisplayName(), NULL);
        addToWidget( _actions, _toolbar);
    }   // end if
    return _toolbar;
}   // end createToolBar


void FaceActionGroup::addTo( QMenu* menu) const
{
    if ( !menu->actions().empty())
        menu->addSeparator();
    addToWidget( _actions, menu);
}   // end addTo


void FaceActionGroup::addTo( QToolBar* toolbar) const
{
    if ( !toolbar->actions().empty())
        toolbar->addSeparator();
    addToWidget( _actions, toolbar);
}   // end addTo


// public virtual
QStringList FaceActionGroup::getInterfaceIds() const
{
    QStringList qlist;
    typedef std::pair<std::string, FaceActionInterface*> FAPair;
    foreach ( const FAPair& fa, _actions)
        qlist << fa.first.c_str();
    return qlist;
}   // end getInterfaceIds


// public virtual
FaceActionInterface* FaceActionGroup::getInterface( const QString& iname) const
{
    const std::string nm = iname.toStdString();
    if ( _actions.count(nm) == 0)
        return NULL;
    return _actions.at(nm);
}   // end getInterface





// public
FaceAction::FaceAction()
    : _action(this), _doasync(false), _pupdater(NULL), _fmaw(NULL)
{
    _action.setText( this->getDisplayName());
    const QIcon* icon = this->getIcon();
    if ( icon)
        _action.setIcon(*icon);
    const QKeySequence* keys = this->getShortcut();
    if ( keys)
        _action.setShortcut(*keys);

    connect( &_action, SIGNAL(triggered()), this, SLOT(process()));
}   // end ctor


// public
FaceAction::~FaceAction()
{
    if ( _fmaw)
    {
        _fmaw->wait();
        delete _fmaw;
    }   // end if
}   // end dtor


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
    tb->connect( tb, SIGNAL(clicked()), &_action, SIGNAL(triggered()));
}   // end connectToolButton


// public
void FaceAction::connectButton( QPushButton* b)
{
    b->addAction( &_action);
    b->setIcon( _action.icon());
    b->setMinimumSize( QSize(26,26));
    b->connect( b, SIGNAL(clicked()), &_action, SIGNAL(triggered()));
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
        return doAction();  // Blocks

    _fmaw = new FaceActionWorker( this);
    connect( _fmaw, SIGNAL(finished(bool)), this, SLOT( workerFinished(bool)));
    _fmaw->run();   // Asynchronous; immediately return and listen for finished(bool)
    return true;
}   // end process


// private slot
void FaceAction::workerFinished(bool rv)
{
    delete _fmaw;
    _fmaw = NULL;
    emit finished(rv);
}   // end workerFinished
