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

#ifndef FACE_TOOLS_ACTION_EXPORT_PDF_H
#define FACE_TOOLS_ACTION_EXPORT_PDF_H

#include "FaceAction.h"
#include <ReportChooserDialog.h>
#include <Report.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    ActionExportPDF( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Generate and export a PDF containing information about the selected model.";}

    // By default, the user is simply asked to save the report. If given a program name here,
    // the report will be opened in the chosen reader (as a forked process).
    void setOpenOnSave( const QString& pdfreader) { _pdfreader = pdfreader;}

protected:
    void postInit() override;
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    void doAfterAction( Event) override;

private:
    Widget::ReportChooserDialog *_dialog;
    Report::Report::Ptr _report;
    QTemporaryDir _tmpdir;
    QString _tmpfile;
    QString _err;
    QString _pdfreader;
};  // end class

}}   // end namespace

#endif
