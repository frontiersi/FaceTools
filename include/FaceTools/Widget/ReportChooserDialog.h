/************************************************************************
 * Copyright (C) 2018 SIS Research Ltd & Richard Palmer
 *
 * Cliniface is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cliniface is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACETOOLS_WIDGET_REPORT_CHOOSER_DIALOG_H
#define FACETOOLS_WIDGET_REPORT_CHOOSER_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QDialog>

namespace Ui { class ReportChooserDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ReportChooserDialog : public QDialog
{ Q_OBJECT
public:
    explicit ReportChooserDialog( QWidget *parent = nullptr);
    ~ReportChooserDialog() override;

    QString selectedReportName() const;

    bool show( const FM*);

private:
    Ui::ReportChooserDialog *_ui;
    //void setRowShown( int, bool);
};  // end class

}}  // end namespaces

#endif
