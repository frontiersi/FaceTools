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

#include <Action/ActionNewerVersionFinder.h>
using FaceTools::Action::ActionNewerVersionFinder;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


ActionNewerVersionFinder::ActionNewerVersionFinder( const QString& dname, const QIcon& icon, const QKeySequence& ks)
    : FaceAction( dname, icon, ks), _updater(nullptr)
{
}   // end ctor


void ActionNewerVersionFinder::postInit()
{
    QWidget* p = static_cast<QWidget*>(parent());
    _updater = new QTools::NetworkUpdater(this);
    connect( _updater, &QTools::NetworkUpdater::onFinishedManifestDownload,
                 this, &ActionNewerVersionFinder::_doOnFinishedManifestDownload);
}   // end postInit


bool ActionNewerVersionFinder::isAllowed( Event) { return !_manifestUrl.isEmpty();}


void ActionNewerVersionFinder::_doOnFinishedManifestDownload( bool v)
{
    if ( v)
    {
        _uvers = _updater->version();
        if ( _cvers >= _uvers)
        {
            static const QString MSG = tr("No new version found.");
            MS::showStatus( MSG, 5000);
            std::cerr << MSG.toStdString() << std::endl;
        }   // end else
        emit onFoundNewerVersion( _uvers > _cvers);
    }   // end if
    else
    {
        const QString msg = QString( "Error downloading manifest: %1").arg(_updater->error());
        MS::showStatus( msg, 5000);
        std::cerr << msg.toStdString() << std::endl;
    }   // end else
}   // end _doOnFinishedManifestDownload


void ActionNewerVersionFinder::doAction( Event) { _updater->downloadManifest( _manifestUrl);}


Event ActionNewerVersionFinder::doAfterAction( Event)
{
    MS::showStatus( "Checking online for update...", 5000);
    return Event::NONE;
}   // end doAfterAction
