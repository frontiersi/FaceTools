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
#include <Widget/ReportChooserDialog.h>
#include <Report/Report.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    ActionExportPDF( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Generate and export a PDF containing information about the selected model.";}

    // By default, the user is simply asked to save the report. If set true here,
    // the report will be opened in the user's default PDF reader after saving.
    static void setOpenOnSave( bool v) { _openOnSave = v;}

    // Returns true iff report generation (in general) is available for the given model.
    // Checks that the U3D cache has a filepath for the given model and that
    // ReportManager::isAvailable returns true.
    static bool isAvailable( const FM*);

    // Ask user where to save a report generated at the given temporary file location.
    // Before returning true, if the PDF viewer is set to open generated reports automatically,
    // the corresponding viewer program will be forked to try to open the file in the newly saved location.
    // Returns false if unable to save the report or if the user cancels.
    static bool saveGeneratedReport( const QString& tmpfile, const QWidget* prnt=nullptr);

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
    static bool _openOnSave;
};  // end class

}}   // end namespace

#endif
