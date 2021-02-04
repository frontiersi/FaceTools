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

#include <Widget/LandmarksCheckDialog.h>
#include <ui_LandmarksCheckDialog.h>
#include <Widget/CheckAllTableHeader.h>
#include <Widget/IntTableWidgetItem.h>
#include <LndMrk/LandmarksManager.h>
#include <LndMrk/LandmarkSet.h>

using FaceTools::Widget::LandmarksCheckDialog;
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


LandmarksCheckDialog::LandmarksCheckDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::LandmarksCheckDialog)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Overwrite Landmarks?");
    setModal(true);

    _ui->table->setColumnCount(3);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table);
    _ui->table->setHorizontalHeader(header);
    connect( header, &CheckAllTableHeader::allChecked, this, &LandmarksCheckDialog::_doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &LandmarksCheckDialog::_doSortOnColumn);

    _populate();

    header->setSortIndicatorShown(false);
    header->setStretchLastSection(true);   // Resize width of final column
    header->setAllChecked(true);
    header->setMinimumSectionSize(14);

    _ui->table->setSelectionMode( QAbstractItemView::NoSelection);
    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::NoFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    _ui->table->setColumnHidden( IDNT_COL, true);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _ui->table->setColumnWidth( SHOW_COL, 18);

    connect( _ui->table, &QTableWidget::itemChanged, this, &LandmarksCheckDialog::_doOnItemChanged);
}   // end ctor


LandmarksCheckDialog::~LandmarksCheckDialog() { delete _ui;}


void LandmarksCheckDialog::_doOnItemChanged( QTableWidgetItem* m)
{
    int id = _ui->table->item( m->row(), IDNT_COL)->text().toInt();
    if ( m->column() == SHOW_COL)
    {
        if ( m->checkState() == Qt::Checked)
            _landmarks.insert(id);
        else
            _landmarks.erase(id);
    }   // end if
}   // end _doOnItemChanged


bool LandmarksCheckDialog::open( const FM &fm)
{
    CheckAllTableHeader *header = static_cast<CheckAllTableHeader*>(_ui->table->horizontalHeader());
    header->setAllChecked(true);
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        const int id = _ui->table->item( i, IDNT_COL)->text().toInt();
        // If the model doesn't have the landmark already set, disable the check button.
        Qt::ItemFlags flags = Qt::ItemIsUserCheckable;
        if ( fm.currentAssessment()->landmarks().has(id))
            flags |= Qt::ItemIsEnabled;

        auto* item = _ui->table->item(i, SHOW_COL);
        item->setFlags( flags);
        item->setCheckState( Qt::Checked);
        _landmarks.insert(id);
    }   // end for
    return QDialog::exec() > 0;
}   // end open


void LandmarksCheckDialog::_doOnSetAllChecked( bool c)
{
    const Qt::CheckState cstate = c ? Qt::Checked : Qt::Unchecked;
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        auto* item = _ui->table->item(i, SHOW_COL);
        if ( item->flags() & Qt::ItemIsEnabled)
            item->setCheckState( cstate);
    }   // end for
}   // end _doOnSetAllChecked


void LandmarksCheckDialog::_populate()
{
    _ui->table->clearContents();
    for ( int id : Landmark::LandmarksManager::ids())
        _appendRow( id);

    _doSortOnColumn( NAME_COL);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();
}   // end _populate


void LandmarksCheckDialog::_appendRow(int id)
{
    Landmark::Landmark* lmk = Landmark::LandmarksManager::landmark(id);
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
}   // end _appendRow


void LandmarksCheckDialog::_doSortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    _idRows.clear();    // Re-map IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
        _idRows[mid] = i;
    }   // end for
}   // end _doSortOnColumn
