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

#ifndef FACE_TOOLS_ACTION_NEWER_VERSION_FINDER_H
#define FACE_TOOLS_ACTION_NEWER_VERSION_FINDER_H

#include "FaceAction.h"
#include <QTools/NetworkUpdater.h>
#include <QUrl>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionNewerVersionFinder : public FaceAction
{ Q_OBJECT
public:
    ActionNewerVersionFinder( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    void setManifestUrl( const QUrl &mu) { _manifestUrl = mu;}
    void setCurrentVersion( const QTools::VersionInfo &v) { _cvers = _uvers = v;}
    const QTools::VersionInfo &updatedVersion() const { return _uvers;}

    QString toolTip() const override { return "Check if a newer version is available to download.";}

signals:
    void onFoundNewerVersion( bool);

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private slots:
    void _doOnFinishedManifestDownload(bool);

private:
    QTools::NetworkUpdater *_updater;
    QTools::VersionInfo _cvers, _uvers;
    QUrl _manifestUrl;
};  // end class

}}   // end namespace

#endif
