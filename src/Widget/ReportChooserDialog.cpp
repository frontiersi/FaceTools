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

#include <Widget/ReportChooserDialog.h>
#include <ui_ReportChooserDialog.h>
#include <Report/ReportManager.h>
#include <U3DCache.h>
#include <QListView>
#include <QStandardItemModel>
using FaceTools::Widget::ReportChooserDialog;
using FaceTools::Report::ReportManager;
using FaceTools::FM;


ReportChooserDialog::ReportChooserDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::ReportChooserDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Report Template Selection");
}   // end ctor


ReportChooserDialog::~ReportChooserDialog()
{
    delete _ui;
}   // end dtor


QString ReportChooserDialog::selectedReportName() const
{
    return _ui->reportsComboBox->currentText();
}   // end selectedReportName


bool ReportChooserDialog::show( const FM* fm)
{
    _ui->reportsComboBox->clear();
    const QStringList& reportNames = ReportManager::names();
    for ( const QString& reportName : reportNames)
    {
        if ( ReportManager::report(reportName)->isAvailable(fm))
            _ui->reportsComboBox->addItem(reportName);
    }   // end for
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
