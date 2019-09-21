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

#include <Widget/PhenotypesDialog.h>
#include <ui_PhenotypesDialog.h>
#include <Widget/IntTableWidgetItem.h>
#include <Metric/MetricCalculatorManager.h>
#include <Metric/SyndromeManager.h>
#include <Metric/PhenotypeManager.h>
#include <FileIO.h>
#include <algorithm>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::PhenotypesDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using FaceTools::Metric::Phenotype;
using SynMan = FaceTools::Metric::SyndromeManager;
using MCM = FaceTools::Metric::MetricCalculatorManager;

namespace {
enum ColIndex
{
    ID_COL = 0,      // ID of phenotype
    NAME_COL = 1,    //
    REGION_COL = 2,  // Region on body
    METRICS_COL = 3, // Names of associated metrics
};  // end enum
}   // end namespace


PhenotypesDialog::PhenotypesDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::PhenotypesDialog), _chid(-1)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Human Phenotype Ontology (HPO) Terms");

    _ui->table->setColumnCount(4);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"ID", "Common Name", "Anatomical Region", "Related Metrics"}));

    QHeaderView* header = _ui->table->horizontalHeader();
    connect( header, &QHeaderView::sectionClicked, this, &PhenotypesDialog::sortOnColumn);

    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::StrongFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    header->setSortIndicatorShown(false);

    connect( _ui->syndromesComboBox, QOverload<int>::of(&QComboBox::activated), this, &PhenotypesDialog::doOnUserSelectedSyndrome);
    header->setStretchLastSection(true);   // Resize width of final column

    connect( _ui->table, &QTableWidget::cellClicked, this, &PhenotypesDialog::highlightRow);
    connect( _ui->table, &QTableWidget::currentItemChanged, [this]( QTableWidgetItem* item){ highlightRow(item->row());});

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // Initial populate
    _dhids = HPOMan::ids();
    for ( int hid : _dhids)
        appendRow( hid);

    populateSyndromes( SynMan::ids());
    sortOnColumn( NAME_COL);
    _ui->table->setCurrentCell(0,0);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->setFocus();
    _ui->table->scrollToTop();
}   // end ctor


PhenotypesDialog::~PhenotypesDialog() { delete _ui;}


// private
void PhenotypesDialog::setSelectedPhenotype( int pid)
{
    const int rowid = pid >= 0 ? _idRows.at(pid) : -1;
    highlightRow(rowid);
}   // end setSelectedPhenotype


// private
void PhenotypesDialog::sortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    bool ok;
    _idRows.clear();    // Re-map IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int pid = _ui->table->item( i, ID_COL)->text().toInt( &ok);
        assert(ok);
        _idRows[pid] = i;
    }   // end for
}   // end sortOnColumn


// private
void PhenotypesDialog::highlightRow( int rowid)
{
    // Clear the previous row
    if ( _chid >= 0)
    {
        assert(_idRows.count(_chid) > 0);
        int orid = _idRows.at(_chid);
        _ui->table->item(orid, ID_COL)->setBackgroundColor( Qt::white);
        _ui->table->item(orid, NAME_COL)->setBackgroundColor( Qt::white);
        _ui->table->item(orid, REGION_COL)->setBackgroundColor( Qt::white);
        _ui->table->item(orid, METRICS_COL)->setBackgroundColor( Qt::white);
    }   // end if

    _chid = -1;
    _ui->remarksTextEdit->clear();
    _ui->criteriaTextEdit->clear();

    if ( rowid >= 0)
    {
        _chid = _ui->table->item(rowid, ID_COL)->text().toInt();

        static const QColor bg(200,235,255);
        _ui->table->item(rowid, ID_COL)->setBackgroundColor( bg);
        _ui->table->item(rowid, NAME_COL)->setBackgroundColor( bg);
        _ui->table->item(rowid, REGION_COL)->setBackgroundColor( bg);
        _ui->table->item(rowid, METRICS_COL)->setBackgroundColor( bg);

        Phenotype::Ptr hpo = HPOMan::phenotype( _chid);
        _ui->criteriaTextEdit->setPlainText( hpo->criteria());

        QString remarks;
        if ( !hpo->remarks().isEmpty())
            remarks = hpo->remarks() + "\n\n";
        remarks += "Synonyms: " + hpo->synonyms().join("; ");
        _ui->remarksTextEdit->setPlainText( remarks);
    }   // end if
}   // end highlightRow


// private
void PhenotypesDialog::populateSyndromes( const IntSet& sids)
{
    _ui->syndromesComboBox->clear();
    _ui->syndromesComboBox->addItem( "-- Any --", -1);

    std::unordered_map<QString, int> nids;
    QStringList slst;
    for ( int sid : sids)
    {
        QString nm = SynMan::syndrome(sid)->name();
        slst.append(nm);
        nids[nm] = sid;
    }   // end for

    slst.sort();
    for ( const QString& nm : slst)
        _ui->syndromesComboBox->addItem( nm, nids.at(nm));

    _ui->syndromesComboBox->setEnabled( _ui->syndromesComboBox->count() > 1);
}   // end populateSyndromes


// private
void PhenotypesDialog::appendRow( int hid)
{
    Phenotype::Ptr mc = HPOMan::phenotype( hid);

    const int rowid = _idRows[hid] = _ui->table->rowCount();
    _ui->table->insertRow(rowid);

    // Id
    QTableWidgetItem* iitem = new IntTableWidgetItem( hid, 5);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, ID_COL, iitem);

    // Name
    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    // Region
    QTableWidgetItem* citem = new QTableWidgetItem( mc->region());
    citem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, REGION_COL, citem);

    // Associated metrics
    QStringList mnames;
    for ( int mid : mc->metrics())
    {
        if ( MCM::metric(mid))
            mnames.append( MCM::metric(mid)->name());
        else
            mnames.append( QString("[%1]").arg(mid));   // Just display the number for currently missing metrics
    }   // end for
    QTableWidgetItem* mitem = new QTableWidgetItem( mnames.join("; "));
    mitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, METRICS_COL, mitem);

    _ui->table->selectRow( rowid);
}   // end appendRow


// private slot
void PhenotypesDialog::doOnUserSelectedSyndrome()
{
    const IntSet *hset = &HPOMan::ids();
    const int sid = _ui->syndromesComboBox->currentData().toInt();
    if ( sid >= 0)
        hset = &SynMan::syndrome(sid)->hpos();

    // Show/hide HPO term table rows according to if the name of the term is in the set for the selected syndrome.
    int shid = -1;
    int rowCount = _ui->table->rowCount();
    for ( int i = 0; i < rowCount; ++i)
    {
        int hid = _ui->table->item(i, ID_COL)->text().toInt();
        if ( (hset->count(hid) > 0) && (_dhids.count(hid) > 0))
        {
            _ui->table->showRow( i);
            if ( shid < 0 || hid == _chid)
                shid = hid;
        }   // end if
        else
            _ui->table->hideRow( i);
    }   // end for

    setSelectedPhenotype(shid);
}   // end doOnUserSelectedSyndrome


void PhenotypesDialog::doOnShowPhenotypes( const IntSet& hids)
{
    int shid = -1;
    IntSet sids;
    int rowCount = _ui->table->rowCount();
    for ( int i = 0; i < rowCount; ++i)
    {
        int hid = _ui->table->item(i, ID_COL)->text().toInt();

        if ( hids.count(hid) > 0)
        {
            _ui->table->showRow( i);
            const IntSet& hsids = SynMan::hpoSyndromes(hid);
            sids.insert( hsids.begin(), hsids.end());
            if ( shid < 0 || hid == _chid)
                shid = hid;
        }   // end if
        else
            _ui->table->hideRow( i);
    }   // end for

    _dhids = hids;
    populateSyndromes(sids);

    setSelectedPhenotype(shid);
}   // end doOnShowPhenotypes
