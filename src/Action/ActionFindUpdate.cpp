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

#include <Action/ActionFindUpdate.h>
#include <QSslSocket>
using FaceTools::Action::ActionFindUpdate;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using MS = FaceTools::Action::ModelSelector;


ActionFindUpdate::ActionFindUpdate( const QString& dname, const QIcon& icon, const QUrl &url)
    : FaceAction( dname, icon), _updater(nullptr), _isUpdate(false), _canConnect(false)
{
    _updater = new QTools::NetworkUpdater( url, 10000, 5);   // 10 second timeout, 5 redirects
    connect( _updater, &QTools::NetworkUpdater::onDownloadProgress, this, &ActionFindUpdate::_doOnDownloadProgress);
    connect( _updater, &QTools::NetworkUpdater::onReplyFinished, this, &ActionFindUpdate::_doOnReplyFinished);
    const QString sslversion = QSslSocket::sslLibraryBuildVersionString();
    std::cerr << sslversion.toStdString() << " " << std::boolalpha << QSslSocket::supportsSsl() << std::endl;
}   // end ctor


bool ActionFindUpdate::update( Event)
{
    _canConnect = _updater->isAvailable();
    return _canConnect;
}   // end update


bool ActionFindUpdate::isAllowed( Event) { return !_updater->isBusy() && _canConnect;}


void ActionFindUpdate::downloadUpdate( const QString &aname)
{
    _isUpdate = true;
    if ( !_updater->downloadUpdate( aname))
        _handleError();
    else
        std::cerr << "Downloading update from " << _updater->updateMeta().updateUrl().toDisplayString().toStdString() << std::endl;
}   // end downloadUpdate


void ActionFindUpdate::_doOnDownloadProgress( qint64 br, qint64 bt)
{
    emit onDownloadProgress( br, bt);
}   // end _doOnDownloadProgress


void ActionFindUpdate::_doOnReplyFinished( bool v)
{
    if ( _isUpdate)
    {
        _isUpdate = false;
        if ( v)
            emit onUpdateDownloaded();
        else
            _handleError();
    }   // end if
    else
        _updateDownloadedManifest(v);
}   // end _doOnReplyFinished


void ActionFindUpdate::_updateDownloadedManifest( bool v)
{
    if ( v)
    {
        if ( _cmeta >= _updater->updateMeta())
            std::cerr << "No update found." << std::endl;
        emit onFoundUpdate( _updater->updateMeta() > _cmeta);
    }   // end if
    else
        _handleError();
}   // end _updateDownloadedManifest


void ActionFindUpdate::_handleError()
{
    const QString err = _updater->error();
    std::cerr << err.toStdString() << std::endl;
    emit onError( err);
}   // end _handleError


void ActionFindUpdate::doAction( Event)
{
    const std::string murl = _updater->manifestUrl().toDisplayString().toStdString();
    std::cerr << "Refreshing manifest from " << murl << std::endl;
    _updater->refreshManifest();
}   // end doAction


Event ActionFindUpdate::doAfterAction( Event)
{
    MS::showStatus( "Checking for update...", 2000);
    return Event::NONE;
}   // end doAfterAction
