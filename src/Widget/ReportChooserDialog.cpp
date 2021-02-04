/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Widget/ReportChooserDialog.h>
#include <ui_ReportChooserDialog.h>
#include <Report/ReportManager.h>
#include <U3DCache.h>
#include <QListView>
#include <QStandardItemModel>
using FaceTools::Widget::ReportChooserDialog;
using RMAN = FaceTools::Report::ReportManager;


ReportChooserDialog::ReportChooserDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::ReportChooserDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Report Selection");
}   // end ctor


ReportChooserDialog::~ReportChooserDialog()
{
    delete _ui;
}   // end dtor


QString ReportChooserDialog::selectedReportName() const
{
    return _ui->reportsComboBox->currentText();
}   // end selectedReportName


bool ReportChooserDialog::show()
{
    _ui->reportsComboBox->clear();
    for ( const QString& reportName : RMAN::names())
        if ( RMAN::report(reportName)->isAvailable())
            _ui->reportsComboBox->addItem(reportName);
    return QDialog::exec() > 0;
}   // end show


/*
// Hide/show rows in the reports combo box
void ReportChooserDialog::setRowShown( int row, bool v)
{
    QListView* view = qobject_cast<QListView*>(_ui->reportsComboBox->view());
    Q_ASSERT( view != nullptr);
    view->setRowHidden( row, !v);

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(_ui->reportsComboBox->model());
    Q_ASSERT( model != nullptr);
    QStandardItem* item = model->item(row);
    Qt::ItemFlags flags = item->flags();
    if ( v)
        flags |= Qt::ItemIsEnabled;
    else
        flags &= ~Qt::ItemIsEnabled;
    item->setFlags(flags);
}   // end setRowShown
*/
