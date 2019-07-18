/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_EXPORT_METADATA_H
#define FACE_TOOLS_ACTION_EXPORT_METADATA_H

#include "FaceAction.h"
#include <QMimeDatabase>
#include <QFileDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportMetaData : public FaceAction
{ Q_OBJECT
public:
    ActionExportMetaData( const QString&, const QIcon&);

    QString toolTip() const override;

protected:
    void postInit() override;
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    void doAfterAction( Event) override;

private:
    QFileDialog *_fdialog;
    QString _filename;
    std::ofstream _ofs;
    QMimeDatabase _mimeDB;
    QStringList _mimefilters;
    QString _getFileName( const FM*);
};  // end class

}}   // end namespace

#endif
