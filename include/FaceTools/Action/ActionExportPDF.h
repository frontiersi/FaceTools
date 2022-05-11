/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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
#include <FaceTools/Widget/ReportChooserDialog.h>
#include <FaceTools/Report/Report.h>
#include <QFileDialog>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExportPDF : public FaceAction
{ Q_OBJECT
public:
    ActionExportPDF( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Generate a PDF report about the selected model.";}

    // By default, the user is simply asked to save the report. If set true here,
    // the report will be opened in the user's default PDF reader after saving.
    static void setOpenOnSave( bool v) { _openOnSave = v;}

    // Returns true iff report generation (in general) is available.
    // Checks that the U3D cache has a filepath for the loaded model(s)
    // and that ReportManager::isAvailable returns true.
    static bool isAvailable();

    // Ask user where to save a report generated at the given temporary file location.
    // Before returning true, if the PDF viewer is set to open reports automatically,
    // the corresponding PDF viewer program will be used to open the report at save location.
    // Returns false if unable to save the report or if the user cancels.
    static bool saveGeneratedReport( const QString& tmpfile, QFileDialog*);

    // Create a save file dialog suitably configured for saving generated reports.
    static QFileDialog* createSaveDialog( QWidget* parent);

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    QFileDialog *_fileDialog;
    Widget::ReportChooserDialog *_dialog;
    Report::Report::Ptr _report;
    QString _err;
    static bool _openOnSave;
    static bool _allowExport;
};  // end class

}}   // end namespace

#endif
