/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_EXPORT_PDF_H
#define FACE_TOOLS_ACTION_EXPORT_PDF_H

#include "FaceAction.h"
#include <ReportChooserDialog.h>
#include <Report.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    ActionExportPDF( const QString& dname="Create Report", const QIcon& icon=QIcon(), QWidget* parent=nullptr);

    // By default, the user is simply asked to save the report. If given a program name here,
    // the report will be opened in the chosen reader (as a forked process).
    void setOpenOnSave( const QString& pdfreader) { _pdfreader = pdfreader;}

protected slots:
    bool testReady( const Vis::FV*) override;
    bool testEnabled( const QPoint* mc=nullptr) const override;
    bool doBeforeAction( FVS&, const QPoint&) override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet&, const FVS&, bool) override;

private:
    Widget::ReportChooserDialog *_dialog;
    QWidget *_parent;
    Report::Report::Ptr _report;
    QTemporaryDir _tmpdir;
    QString _tmpfile;
    QString _err;
    QString _pdfreader;
};  // end class

}}   // end namespace

#endif
