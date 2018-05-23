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
#include <sstream>
#include <cassert>
#include <iomanip>
using FaceTools::Action::FaceActionManager;
using FaceTools::Action::FaceActionGroup;
using FaceTools::Action::ChangeEventSet;
using FaceTools::Action::ActionSelect;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;

// public
FaceActionManager::FaceActionManager( QWidget* parent)
    : QObject(), _pdialog( new QTools::PluginsDialog(parent))
{
    _selector.setExternalSelect(false); // the selection events this action produces should not be fed back to it!
    connect( &_selector, &ActionSelect::onSelect, this, &FaceActionManager::doOnSelect);
    connect( &_selector, &ActionSelect::onRemove, this, &FaceActionManager::doOnRemove);
    addAction( &_selector); // Selector will respond to data changes that cause the selection outline to change
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


// public
QAction* FaceActionManager::addAction( FaceAction* faction)
{
    assert( faction);
    if ( _actions.count(faction) > 0)
        return NULL;

    faction->init();
    connect( faction, &FaceAction::reportStarting, this, &FaceActionManager::doOnActionStarting);
    connect( faction, &FaceAction::reportFinished, this, &FaceActionManager::doOnActionFinished);
    connect( faction, &FaceAction::reportChanges,  this, &FaceActionManager::doOnReportChanges);

    _actions.insert( faction);
    return faction->qaction();
}   // end addAction


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
{
    _selector.setSelectEnabled(false);  // Prevent new selection events from firing
    // Disable actions upon an action starting. The initiating action disables itself.
    std::for_each( std::begin(_actions), std::end(_actions),
            [=](auto a)
            {
                if ( a->isDisabledBeforeOther())
                    a->setEnabled(false);
            });
}   // end doOnActionStarting


// private slot
void FaceActionManager::doOnActionFinished()
{
    // Set the selected FaceControls on the actions again
    for ( FaceControl* fc : _selector.selected())
        doOnSelect( fc, true);
    std::for_each( std::begin(_actions), std::end(_actions), [=](auto a){ a->setEnabled(a->testEnabled());});
    if ( _selector.selected().empty())
        std::for_each( std::begin(_actions), std::end(_actions), [=](auto a){ a->setChecked( false);});
    _selector.setSelectEnabled(true);   // Re-enable selection events
}   // end doOnActionFinished


// private slot
void FaceActionManager::doOnReportChanges( const ChangeEventSet& cset, FaceControl* fc)
{
    FaceAction* sending = qobject_cast<FaceAction*>(sender());
    _actions.erase(sending);    // Sender mustn't respond to self!
    for ( FaceAction* a : _actions)
    {
        for ( const auto& c : cset)
        {
            if ( a->respondEvents().count(c))
            {
                a->respondTo( sending, &cset, fc);
                break;
            }   // end if
        }   // end for
    }   // end for
    _actions.insert(sending);
}   // end doOnReportChanges


// private slot
void FaceActionManager::doOnSelect( FaceControl* fc, bool v)
{
    // Tell actions that are responsive to selection events to set the ready state for the given FaceControl.
    std::for_each( std::begin(_actions), std::end(_actions), [=](auto a)
            {
                if ( a->externalSelect())
                    a->setSelected( fc, v);
            });
    std::for_each( std::begin(_actions), std::end(_actions), [=](auto a){ a->setEnabled(a->testEnabled());});
}   // end doOnSelect


// private slot
void FaceActionManager::doOnRemove( FaceControl* fc)
{
    std::for_each( std::begin(_actions), std::end(_actions), [=](auto a){ a->purge( fc);});
}   // end doOnRemove

