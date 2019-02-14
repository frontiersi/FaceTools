/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#include <ReportChooserDialog.h>
#include <ui_ReportChooserDialog.h>
#include <U3DCache.h>
#include <ReportManager.h>
using FaceTools::Widget::ReportChooserDialog;
using FaceTools::Report::ReportManager;


ReportChooserDialog::ReportChooserDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::ReportChooserDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Select Report");
    _ui->reportsComboBox->addItems( ReportManager::names());
}   // end ctor


ReportChooserDialog::~ReportChooserDialog()
{
    delete _ui;
}   // end dtor


QString ReportChooserDialog::selectedReportName() const
{
    return _ui->reportsComboBox->currentText();
}   // end selectedReportName

