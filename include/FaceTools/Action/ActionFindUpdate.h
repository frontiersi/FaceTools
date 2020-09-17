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

#ifndef FACE_TOOLS_ACTION_FIND_UPDATE_H
#define FACE_TOOLS_ACTION_FIND_UPDATE_H

#include "FaceAction.h"
#include <QTools/NetworkUpdater.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionFindUpdate : public FaceAction
{ Q_OBJECT
public:
    ActionFindUpdate( const QString&, const QIcon&, const QUrl &manifestUrl);

    QString toolTip() const override { return "Check if an update is available to download.";}

    void setCurrentMeta( const QTools::UpdateMeta &v) { _cmeta = v;}
    const QTools::UpdateMeta &updateMeta() const { return _updater->updateMeta();}
    void downloadUpdate( const QString&);  // Download archive file to given location

signals:
    void onFoundUpdate( bool);    // Parameter is true iff an update is available
    void onDownloadProgress( qint64, qint64);
    void onUpdateDownloaded();
    void onError( const QString&);      // On any download error

protected:
    bool isAllowed( Event) override;
    bool update( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private slots:
    void _doOnReplyFinished(bool);

private:
    void _updateDownloadedManifest(bool);
    QTools::NetworkUpdater *_updater;
    QTools::UpdateMeta _cmeta;
    bool _isUpdate;
    bool _canConnect;
    void _handleError();
};  // end class

}}   // end namespace

#endif
