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

#include <Widget/DetectionCheckDialog.h>
#include <ui_DetectionCheckDialog.h>
#include <Widget/CheckAllTableHeader.h>
#include <Widget/IntTableWidgetItem.h>
#include <LndMrk/LandmarksManager.h>
#include <LndMrk/LandmarkSet.h>
#include <FaceModel.h>
using FaceTools::Widget::DetectionCheckDialog;
using FaceTools::Widget::CheckAllTableHeader;
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Landmark::Landmark;
using FaceTools::FM;

namespace {
enum ColIndex
{
    SHOW_COL = 0,
    IDNT_COL = 1,
    NAME_COL = 2
};  // end enum
}   // end namespace


DetectionCheckDialog::DetectionCheckDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::DetectionCheckDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Overwrite Detection?");
    setModal(true);

    _ui->table->setColumnCount(3);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table);
    _ui->table->setHorizontalHeader(header);
    connect( header, &CheckAllTableHeader::allChecked, this, &DetectionCheckDialog::doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &DetectionCheckDialog::sortOnColumn);
    header->setStretchLastSection(true);   // Resize width of final column
    header->setAllChecked(true);

    _ui->table->setSelectionMode( QAbstractItemView::NoSelection);
    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::NoFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    header->setSortIndicatorShown(false);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    populate();

    connect( _ui->table, &QTableWidget::itemChanged, this, &DetectionCheckDialog::doOnItemChanged);
}   // end ctor


DetectionCheckDialog::~DetectionCheckDialog()
{
    delete _ui;
}   // end dtor


void DetectionCheckDialog::doOnItemChanged( QTableWidgetItem* m)
{
    int id = _ui->table->item( m->row(), IDNT_COL)->text().toInt();
    if ( m->column() == SHOW_COL)
    {
        if ( m->checkState() == Qt::Checked)
            _landmarks.insert(id);
        else
            _landmarks.erase(id);
    }   // end if
}   // end doOnItemChanged


bool DetectionCheckDialog::open( const FM* fm)
{
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        const int id = _ui->table->item( i, IDNT_COL)->text().toInt();
        // If the model doesn't have the landmark already set, disable the check button.
        Qt::ItemFlags flags = Qt::ItemIsUserCheckable;
        if ( fm->currentAssessment()->landmarks().has(id))
            flags |= Qt::ItemIsEnabled;

        auto* item = _ui->table->item(i, SHOW_COL);
        item->setFlags( flags);
        item->setCheckState( Qt::Checked);
        _landmarks.insert(id);
    }   // end for
    return QDialog::exec() > 0;
}   // end open


void DetectionCheckDialog::doOnSetAllChecked( bool c)
{
    const Qt::CheckState cstate = c ? Qt::Checked : Qt::Unchecked;
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        auto* item = _ui->table->item(i, SHOW_COL);
        if ( item->flags() & Qt::ItemIsEnabled)
            item->setCheckState( cstate);
    }   // end for
}   // end doOnSetAllChecked


void DetectionCheckDialog::populate()
{
    _ui->table->clearContents();
    for ( int id : LDMKS_MAN::ids())
        appendRow( id);

    sortOnColumn( NAME_COL);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();
}   // end populate


void DetectionCheckDialog::appendRow(int id)
{
    Landmark::Landmark* lmk = LDMKS_MAN::landmark(id);
    assert(lmk);

    const int rowid = _idRows[id] = _ui->table->rowCount();    // Id of new row to be entered
    _ui->table->insertRow(rowid);

    QTableWidgetItem* iitem = new IntTableWidgetItem( id, 5);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, IDNT_COL, iitem);

    QTableWidgetItem* nitem = new QTableWidgetItem( lmk->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    QTableWidgetItem* vitem = new QTableWidgetItem;
    vitem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    vitem->setCheckState( Qt::Checked);
    _ui->table->setItem( rowid, SHOW_COL, vitem);
}   // end appendRow


void DetectionCheckDialog::sortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    resetIdRowMap();
}   // end sortOnColumn


void DetectionCheckDialog::resetIdRowMap()
{
    _idRows.clear();    // Re-map IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
        _idRows[mid] = i;
    }   // end for
}   // end resetIdRowMap
