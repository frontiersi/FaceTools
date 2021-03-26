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

#include <Widget/LandmarksDialog.h>
#include <ui_LandmarksDialog.h>
#include <Widget/CheckAllTableHeader.h>
#include <Widget/IntTableWidgetItem.h>

#include <LndMrk/LandmarksManager.h>
#include <LndMrk/LandmarkSet.h>
#include <MiscFunctions.h>
#include <ModelSelect.h>
#include <FaceModel.h>
using FaceTools::Action::FaceAction;
using MS = FaceTools::ModelSelect;

#include <QHeaderView>
#include <QToolTip>
#include <QAction>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::LandmarksDialog;
using LMAN = FaceTools::Landmark::LandmarksManager;
using LMK = FaceTools::Landmark::Landmark;

namespace {
enum ColIndex
{
    SHOW_COL = 0,
    IDNT_COL = 1,
    NAME_COL = 2
};  // end enum
}   // end namespace


LandmarksDialog::LandmarksDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::LandmarksDialog), _prowid(-1)
{
    _ui->setupUi(this);
    _ui->msgLabel->setStyleSheet("color: red;");
    _ui->remarksTextBrowser->setOpenExternalLinks(true);

    setWindowTitle( parent->windowTitle() + " | Landmarks Browser");

    //setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint);
    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->table->setColumnCount(3);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table, true/* use eye icon */);
    _ui->table->setHorizontalHeader(header);
    connect( header, &CheckAllTableHeader::allChecked, this, &LandmarksDialog::_doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &LandmarksDialog::_doSortOnColumn);

    _populate();

    header->setSortIndicatorShown(false);
    header->setStretchLastSection(true);   // Resize width of final column
    header->setMinimumSectionSize(14);

    _ui->table->setSelectionMode( QAbstractItemView::SingleSelection);
    _ui->table->setSelectionBehavior( QAbstractItemView::SelectionBehavior::SelectRows);
    _ui->table->setShowGrid(false);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setFocusPolicy( Qt::FocusPolicy::StrongFocus);
    _ui->table->setSortingEnabled(true);
    _ui->table->setColumnHidden( IDNT_COL, true);    // Hide the ID column

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _ui->table->setColumnWidth( SHOW_COL, 18);

    connect( _ui->table, &QTableWidget::currentItemChanged,
             [this]( QTableWidgetItem *item) { _doOnSetTableRow(item->row());});
    connect( _ui->table, &QTableWidget::itemChanged, this, &LandmarksDialog::_doOnItemChanged);

    _doOnSetTableRow( 0);
}   // end ctor



LandmarksDialog::~LandmarksDialog() { delete _ui;}


void LandmarksDialog::setShowAction( FaceAction *act)
{
    _ui->showButton->setVisible(false);
    _ui->showButton->setDefaultAction( act->qaction());
}   // end setShowAction


void LandmarksDialog::_triggerShow( bool c)
{
    for ( QAction *act : _ui->showButton->actions())
    {
        FaceAction *fact = qobject_cast<FaceAction*>( act->parent());
        fact->execute( Action::Event::USER);
    }   // end for
}   // end _triggerShow


void LandmarksDialog::_doOnRefreshTable()
{
    for ( int i = 0; i < _ui->table->rowCount(); ++i)
    {
        if ( !_ui->table->isRowHidden(i))   // Only affects visible rows
        {
            const int id = _ui->table->item( i, IDNT_COL)->text().toInt();
            const Qt::CheckState cstate = LMAN::landmark(id)->isVisible() ? Qt::Checked : Qt::Unchecked;
            _ui->table->item( i, SHOW_COL)->setCheckState( cstate);
        }   // end if
    }   // end for
}   // end _doOnRefreshTable


void LandmarksDialog::setAlignAction( FaceAction *act) { _ui->alignButton->setDefaultAction( act->qaction());}
void LandmarksDialog::setLabelsAction( FaceAction *act) { _ui->labelsButton->setDefaultAction( act->qaction());}


void LandmarksDialog::setMessage( const QString &msg)
{
    _ui->msgLabel->setText( msg);
    _ui->msgLabel->setVisible( !msg.isEmpty());
}   // end setMessage


QString LandmarksDialog::message() const { return _ui->msgLabel->text();}


void LandmarksDialog::show()
{
    QDialog::show();
    raise();
    activateWindow();
    _triggerShow( MS::isViewSelected() && true);
}   // end show


void LandmarksDialog::setSelectedLandmark( int lmid) { _doOnSetTableRow( _idRows[lmid]);}


void LandmarksDialog::showEvent( QShowEvent *e)
{
    positionWidgetToSideOfParent(this);
    QDialog::showEvent(e);
}   // end showEvent


void LandmarksDialog::closeEvent( QCloseEvent* e)
{
    e->accept();
    accept();
}   // end closeEvent


void LandmarksDialog::_doOnSetAllChecked( bool c)
{
    for ( int i = 0; i < _ui->table->rowCount(); ++i)
    {
        if ( !_ui->table->isRowHidden(i))   // Only affects visible rows
        {
            const int id = _ui->table->item( i, IDNT_COL)->text().toInt();
            LMAN::landmark(id)->setVisible(c);
        }   // end if
    }   // end for

    _ui->table->blockSignals(true);
    _doOnRefreshTable();
    _ui->table->blockSignals(false);

    _triggerShow( MS::isViewSelected() && c);
}   // end _doOnSetAllChecked


void LandmarksDialog::_doSortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    _idRows.clear();    // Re-map IDs to row indices
    for ( int i = 0; i < _ui->table->rowCount(); ++i)
        _idRows[_ui->table->item( i, IDNT_COL)->text().toInt()] = i;
}   // end _doSortOnColumn


void LandmarksDialog::_highlightRow( int rowid)
{
    _ui->table->blockSignals(true);

    if ( _prowid >= 0)
    {
        static const QBrush wbg( Qt::white);
        _ui->table->item( _prowid, SHOW_COL)->setBackground( wbg);
        _ui->table->item( _prowid, IDNT_COL)->setBackground( wbg);
        _ui->table->item( _prowid, NAME_COL)->setBackground( wbg);
    }   // end if

    static const QBrush bg( QColor(200,235,255));
    _ui->table->item( rowid, SHOW_COL)->setBackground( bg);
    _ui->table->item( rowid, IDNT_COL)->setBackground( bg);
    _ui->table->item( rowid, NAME_COL)->setBackground( bg);

    _ui->table->setCurrentCell( rowid, SHOW_COL);
    _ui->table->blockSignals(false);
}   // end _highlightRow


void LandmarksDialog::_doOnSetTableRow( int rowid)
{
    _highlightRow( rowid);
    _prowid = rowid;
    const LMK *lmk = LMAN::landmark( _ui->table->item( rowid, IDNT_COL)->text().toInt());
    _ui->remarksTextBrowser->setHtml( lmk->description());
    _ui->imageLabel->setPixmap( lmk->pixmap());
}   // end _doOnSetTableRow


void LandmarksDialog::_doOnItemChanged( QTableWidgetItem* m)
{
    _doOnSetTableRow( m->row());
    bool newVis = false;
    const int id = _ui->table->item( m->row(), IDNT_COL)->text().toInt();
    switch ( m->column())
    {
        case SHOW_COL:
            newVis = m->checkState() == Qt::Checked && !_ui->table->isRowHidden( m->row());
            LMAN::landmark( id)->setVisible( newVis);
            break;
    }   // end switch

    // If any of the landmarks are visible, then trigger on otherwise trigger off
    _triggerShow( MS::isViewSelected() && LMAN::anyLandmarksVisible());
}   // end _doOnItemChanged


void LandmarksDialog::_populate()
{
    _ui->table->clearContents();
    for ( int id : LMAN::ids())
        _appendRow( id);

    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();

    CheckAllTableHeader* header = qobject_cast<CheckAllTableHeader*>(_ui->table->horizontalHeader());
    header->setAllChecked(true);
    _doSortOnColumn( NAME_COL);
}   // end _populate


void LandmarksDialog::_appendRow( int id)
{
    LMK *lmk = LMAN::landmark( id);
    assert( lmk);

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
    vitem->setCheckState( lmk->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->setItem( rowid, SHOW_COL, vitem);
}   // end _appendRow


