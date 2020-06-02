/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <Widget/PhenotypesDialog.h>
#include <ui_PhenotypesDialog.h>
#include <Metric/MetricManager.h>
#include <Metric/SyndromeManager.h>
#include <Metric/PhenotypeManager.h>
#include <rlib/FileIO.h>
#include <algorithm>
#include <QCloseEvent>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QSplitter>
using FaceTools::Widget::PhenotypesDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using SynMan = FaceTools::Metric::SyndromeManager;
using MM = FaceTools::Metric::MetricManager;

namespace {
enum ColIndex
{
    ID_COL = 0,      // ID of phenotype
    NAME_COL = 1,    //
    METRICS_COL = 2, // Names of associated metrics
};  // end enum
}   // end namespace


PhenotypesDialog::PhenotypesDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::PhenotypesDialog), _chid(-1)
{
    _ui->setupUi(this);
    _ui->splitter_2->setStretchFactor( 0,2);
    _ui->splitter_2->setStretchFactor( 1,1);

    setWindowTitle( parent->windowTitle() + " | Human Phenotype Ontology (HPO) Browser");

    _ui->table->setColumnCount(3);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"Term Id", "Term Name", "Related Measurements"}));

    QHeaderView* header = _ui->table->horizontalHeader();
    connect( header, &QHeaderView::sectionClicked, this, &PhenotypesDialog::_sortOnColumn);
    header->setSortIndicatorShown(false);
    header->setStretchLastSection(true);   // Resize width of final column

    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::StrongFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);

    _populateTable();
    _populateSyndromes();
    _populateAnatomicalRegions();

    _ui->table->verticalHeader()->setSectionResizeMode( QHeaderView::ResizeToContents);
    _ui->table->setFocus();
    _ui->table->scrollToTop();
    _sortOnColumn( NAME_COL);
    _ui->table->setCurrentCell(0,0);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();

    connect( _ui->table, &QTableWidget::currentItemChanged,
            [this]( QTableWidgetItem* item){ assert(item); _highlightRow( item->row());});
    connect( _ui->syndromesComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &PhenotypesDialog::_doOnUserSelectedSyndrome);
    connect( _ui->anatomicalRegionsComboBox, QOverload<int>::of(&QComboBox::activated),
            this, &PhenotypesDialog::_doOnUserSelectedAnatomicalRegion);

    _highlightRow(0);
}   // end ctor


PhenotypesDialog::~PhenotypesDialog() { delete _ui;}


void PhenotypesDialog::show()
{
    QDialog::show();
    raise();
    activateWindow();
}   // end show


void PhenotypesDialog::closeEvent( QCloseEvent *e)
{
    e->accept();
    accept();
}   // end closeEvent


void PhenotypesDialog::_sortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    bool ok;
    _idRows.clear();    // Re-map IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int pid = _ui->table->item( i, ID_COL)->data(Qt::UserRole).toInt( &ok);
        assert(ok);
        _idRows[pid] = i;
    }   // end for
}   // end _sortOnColumn


void PhenotypesDialog::_highlightRow( int rowid)
{
    // Clear the previous row
    if ( _chid >= 0)
    {
        assert(_idRows.count(_chid) > 0);
        int orid = _idRows.at(_chid);
        _ui->table->item(orid, ID_COL)->setBackgroundColor( Qt::white);
        _ui->table->item(orid, NAME_COL)->setBackgroundColor( Qt::white);
        _ui->table->item(orid, METRICS_COL)->setBackgroundColor( Qt::white);
    }   // end if

    _chid = -1;
    if ( rowid >= 0)
    {
        static const QColor bg(200,235,255);
        _ui->table->item(rowid, ID_COL)->setBackgroundColor( bg);
        _ui->table->item(rowid, NAME_COL)->setBackgroundColor( bg);
        _ui->table->item(rowid, METRICS_COL)->setBackgroundColor( bg);
        _chid = _ui->table->item(rowid, ID_COL)->data(Qt::UserRole).toInt();
    }   // end if

    QSignalBlocker blocker( _ui->table);
    _ui->table->setCurrentCell( rowid, ID_COL);

    _updateSelectedInfo();
}   // end _highlightRow


void PhenotypesDialog::_updateSelectedInfo()
{
    _ui->remarksTextBrowser->clear();
    _ui->criteriaTextBrowser->clear();
    if ( _chid >= 0)
    {
        Phenotype::Ptr hpo = HPOMan::phenotype( _chid);
        QString criteria;
        if ( !hpo->objectiveCriteria().isEmpty())
            criteria += "<h4>Objective</h4><p>" + hpo->objectiveCriteria() + "</p>";
        if ( !hpo->subjectiveCriteria().isEmpty())
            criteria += "<h4>Subjective</h4><p>" + hpo->subjectiveCriteria() + "</p>";
        _ui->criteriaTextBrowser->setHtml( criteria);

        QString remarks;
        if ( !hpo->remarks().isEmpty())
            remarks += "<p>" + hpo->remarks() + "</p>";
        if ( !hpo->synonyms().isEmpty())
            remarks += "<h4>Synonyms</h4><p>" + hpo->synonyms().join("; ") + "</p>";
        if ( !hpo->refs().isEmpty())
            remarks += "<h4>References</h4><p>" + hpo->refs().join("<br>") + "</p>";
        _ui->remarksTextBrowser->setHtml( remarks);
    }   // end if
}   // end _updateSelectedInfo


void PhenotypesDialog::_populateSyndromes()
{
    _ui->syndromesComboBox->clear();
    _ui->syndromesComboBox->addItem( "-- Any --");
    _ui->syndromesComboBox->addItems( SynMan::names());
    //_ui->syndromesComboBox->setEnabled( SynMan::size() > 0);
    _ui->syndromesComboBox->setEnabled( false); // ENABLE LATER
}   // end _populateSyndromes


void PhenotypesDialog::_populateAnatomicalRegions()
{
    _ui->anatomicalRegionsComboBox->clear();
    _ui->anatomicalRegionsComboBox->addItem( "-- Any --");
    _ui->anatomicalRegionsComboBox->addItems( HPOMan::regions());
    _ui->anatomicalRegionsComboBox->setEnabled( HPOMan::size() > 0);
}   // end _populateAnatomicalRegions


void PhenotypesDialog::_doOnUserSelectedSyndrome()
{
    _setFilteredHPOs();
    _refresh();
}   // end _doOnUserSelectedSyndrome


void PhenotypesDialog::_doOnUserSelectedAnatomicalRegion()
{
    _setFilteredHPOs();
    _refresh();
}   // end _doOnUserSelectedAnatomicalRegion


void PhenotypesDialog::_setFilteredHPOs()
{
    _fhpos.clear();
    const int sidx = _ui->syndromesComboBox->currentIndex();
    const IntSet& hids = sidx > 0 ? SynMan::syndrome( _ui->syndromesComboBox->currentText())->hpos() : HPOMan::ids();
    for ( int hid : hids)
        if ( _allhpos.count(hid) > 0)
            _fhpos.insert(hid);

    // Use only the HPOs that match the selected anatomical region
    if ( _ui->anatomicalRegionsComboBox->currentIndex() > 0)
    {
        const IntSet &rhids = HPOMan::byRegion( _ui->anatomicalRegionsComboBox->currentText());
        IntSet mhids;
        for ( int hid : _fhpos)
            if ( rhids.count(hid) > 0)
                mhids.insert(hid);
        _fhpos = mhids;
    }   // end if
}   // end _setFilteredHPOs


void PhenotypesDialog::showPhenotypes( const IntSet& hids)
{
    QSignalBlocker blockerA( _ui->syndromesComboBox);
    QSignalBlocker blockerB( _ui->anatomicalRegionsComboBox);
    _ui->syndromesComboBox->setCurrentIndex( 0);
    _ui->anatomicalRegionsComboBox->setCurrentIndex( 0);
    _allhpos = _fhpos = hids;
    _refresh();
}   // end showPhenotypes


void PhenotypesDialog::_refresh()
{
    int shid = -1;
    int rowCount = _ui->table->rowCount();
    for ( int i = 0; i < rowCount; ++i)
    {
        const int hid = _ui->table->item(i, ID_COL)->data(Qt::UserRole).toInt();
        if ( _fhpos.count(hid) > 0)
        {
            _ui->table->showRow( i);
            if ( shid < 0 || hid == _chid)
                shid = hid;
        }   // end if
        else
            _ui->table->hideRow( i);
    }   // end for

    _highlightRow( shid >= 0 ? _idRows.at(shid) : -1);
}   // end _refresh


void PhenotypesDialog::selectHPO( int shid)
{
    if ( shid >= 0)
        _highlightRow( _idRows.at(shid));
}   // end selectHPO


void PhenotypesDialog::_populateTable()
{
    _allhpos = _fhpos = HPOMan::ids();
    for ( int hid : _allhpos)
        _appendRow( hid);
}   // end _populateTable


void PhenotypesDialog::_appendRow( int hid)
{
    Phenotype::Ptr mc = HPOMan::phenotype( hid);

    const int rowid = _idRows[hid] = _ui->table->rowCount();
    _ui->table->insertRow(rowid);

    // Id
    const QString iname = QString("HP:%1").arg(hid, int(7), int(10), QChar('0'));
    QTableWidgetItem* iitem = new QTableWidgetItem( iname, 0);
    iitem->setData( Qt::UserRole, hid);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, ID_COL, iitem);

    // Name
    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    // Related metrics
    QStringList mnames;
    for ( int mid : mc->metrics())
    {
        if ( MM::metric(mid))
            mnames.append( MM::metric(mid)->name());
        else
            mnames.append( QString("[%1]").arg(mid));   // Just display the number for currently missing metrics
    }   // end for
    QTableWidgetItem* mitem = new QTableWidgetItem( mnames.join("; "));
    mitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, METRICS_COL, mitem);

    _ui->table->selectRow( rowid);
}   // end _appendRow
