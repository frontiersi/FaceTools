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

#include <FaceActionManager.h>
#include <PluginsLoader.h>  // QTools
#include <QApplication>
#include <QString>
#include <algorithm>
#include <cassert>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;

// public
FaceActionManager::FaceActionManager( QWidget* parent)
    : QObject(), _pdialog( new QTools::PluginsDialog(parent))
{
}   // end ctor


// public
FaceActionManager::~FaceActionManager()
{
    delete _pdialog;
}   // end dtor


// public
void FaceActionManager::loadPlugins()
{
    const QString dllsDir = QApplication::applicationDirPath() + "/plugins";
    QTools::PluginsLoader ploader( dllsDir.toStdString());
    std::cerr << "[INFO] FaceTools::Action::FaceActionManager: DLLs directory: "
              << ploader.getPluginsDir().absolutePath().toStdString() << std::endl;
    connect( &ploader, &QTools::PluginsLoader::loadedPlugin, this, &FaceActionManager::addPlugin);
    ploader.loadPlugins();
    _pdialog->addPlugins( ploader);
}   // end loadPlugins


// private
void FaceActionManager::connectActionPair( FaceAction* ract, FaceAction* cact)
{
    const ChangeEventSet& cset = cact->changeEvents();
    const ChangeEventSet& rset = ract->respondEvents();
    for ( auto e : cset)
    {
        if ( rset.count(e) > 0)
        {
            // Default implementation of respondToChange causes the FaceAction to re-evaluate whether
            // the changed FaceControl should be a part of the ready set or not (and then calls setEnabled).
            connect( cact, &FaceAction::reportChanged, ract, &FaceAction::respondToChange);
            break;
        }   // end if
    }   // end for
}   // end connectActionPair


// public
QAction* FaceActionManager::addAction( FaceAction* faction)
{
    assert( faction);
    if ( _actions.count(faction) > 0)
        return NULL;

    faction->init();
    connect( faction, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
    connect( faction, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);

    for ( FaceAction* action : _actions)
    {
        connectActionPair( faction,  action);
        connectActionPair(  action, faction);
    }   // end for
    _actions.insert( faction);
    return faction->qaction();
}   // end addAction


// public slot
void FaceActionManager::setSelected( FaceControl* fc, bool v)
{
    std::for_each( std::begin(_actions), std::end(_actions), [=]( FaceAction* a){ a->setSelected( fc, v);});
}   // end setSelected


// public slot
void FaceActionManager::remove( FaceControl* fc)
{
    std::for_each( std::begin(_actions), std::end(_actions), [=]( FaceAction* a){ a->burn( fc);});
}   // end remove


// private slot
void FaceActionManager::addPlugin( QTools::PluginInterface* plugin)
{
    FaceAction* faction = NULL;
    FaceActionGroup* grp = qobject_cast<FaceActionGroup*>(plugin);
    if ( grp)
    {
        for ( const QString& iid : grp->getInterfaceIds())
        {
            faction = qobject_cast<FaceAction*>( grp->getInterface(iid));
            addAction(faction);
        }   // end for
        emit addedActionGroup( *grp);
    }   // end if
    else if ( faction = qobject_cast<FaceAction*>(plugin))
    {
        addAction( faction);
        emit addedAction( *faction);
    }   // end if
}   // end addPlugin


// private slot
void FaceActionManager::doOnActionStarting( const FaceControlSet* workSet)
{   // Disable all actions (including the starting action) upon some action starting
    assert(workSet);
    FaceAction* sending = qobject_cast<FaceAction*>(sender());
    assert(sending);
    std::for_each( _actions.begin(), _actions.end(), [=]( auto a){ if ( a->isDisabledBeforeOther()) a->setEnabled(false);});
    emit reportStarting( *sending, *workSet);
}   // end doOnActionStarting


// private slot
void FaceActionManager::doOnActionFinished( const FaceControlSet* workedSet)
{
    if ( !workedSet)    // Don't do anything if the action cancelled, just re-enable others (don't check ready set membership)
        std::for_each( std::begin(_actions), std::end(_actions), [=]( auto a){ a->setEnabled(a->testEnabled());});
    else
    {
        FaceAction* sending = qobject_cast<FaceAction*>(sender());
        assert(sending);
        // Allow all the other actions to recheck if the objects in the worked over set
        // should be part of their ready sets.
        for ( FaceAction* a : _actions)
        {
            if ( a != sending)  // Sending action rechecks its ready set first (see FaceAction::doOnActionFinished).
                std::for_each( std::begin(*workedSet), std::end(*workedSet), [&]( auto fc){ a->setSelected( fc, true);});
        }   // end for
        emit reportFinished( *sending, *workedSet);
    }   // end else
}   // end doOnActionFinished
