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

#include <MetricsDisplayDialog.h>
#include <ui_MetricsDisplayDialog.h>
#include <MetricCalculatorManager.h>
#include <SyndromeManager.h>
#include <HPOTermManager.h>
#include <FileIO.h>
#include <algorithm>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QComboBox>
using FaceTools::Widget::MetricsDisplayDialog;
using HPOMan = FaceTools::Metric::HPOTermManager;
using SynMan = FaceTools::Metric::SyndromeManager;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;

namespace {
enum ColIndex
{
    ID_COL = 0,
    NAME_COL = 1,
    CATG_COL = 2,
    DESC_COL = 3,
    SHOW_COL = 4
};  // end enum
}   // end namespace


MetricsDisplayDialog::MetricsDisplayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MetricsDisplayDialog)
{
    ui->setupUi(this);

    ui->table->setColumnCount(5);   // ID, name, category, description, show
    ui->table->setHorizontalHeaderLabels( QStringList( {"ID", "Name", "Category", "Description", "Show"}));

    ui->table->setSelectionMode( QAbstractItemView::SingleSelection);
    ui->table->setSelectionBehavior( QAbstractItemView::SelectRows);
    ui->table->setShowGrid(false);

    connect( ui->table->horizontalHeader(), &QHeaderView::sectionResized, this, &MetricsDisplayDialog::doOnResizeColumns);
    connect( ui->table, &QTableWidget::cellClicked, this, &MetricsDisplayDialog::setRowSelected);
    connect( ui->table, &QTableWidget::itemChanged, this, &MetricsDisplayDialog::doOnRowChanged);

    connect( ui->opacitySpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MetricsDisplayDialog::onChangeOpacity);
    connect( ui->hpoComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDisplayDialog::doOnUserSelectedHPOTerm);
    connect( ui->syndromesComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDisplayDialog::doOnUserSelectedSyndrome);

    ui->table->setStyleSheet("QTableView {selection-background-color: red;}");
    ui->table->setEditTriggers(QAbstractItemView::NoEditTriggers);
}   // end ctor


MetricsDisplayDialog::~MetricsDisplayDialog()
{
    delete ui;
}   // end dtor


void MetricsDisplayDialog::doOnResizeColumns()
{
    QHeaderView* hheader = ui->table->horizontalHeader();
    /*
    static const int CHECKBOX_WIDTH = 30;
    const double PWIDTH = int(ui->table->width()) - CHECKBOX_WIDTH;  // Width to fill

    hheader->resizeSection( ID_COL, 0);
    hheader->resizeSection( NAME_COL, int(0.3*PWIDTH));
    hheader->resizeSection( CATG_COL, int(0.2*PWIDTH));
    hheader->resizeSection( DESC_COL, int(0.4*PWIDTH));
    hheader->resizeSection( SHOW_COL, CHECKBOX_WIDTH);
    */
    hheader->setStretchLastSection(true);   // Resize width of final column
}   // end doOnResizeColumns


void MetricsDisplayDialog::setMetricSelected( int mid)
{
    if ( _mIdRows.count(mid) > 0)
        setRowSelected( _mIdRows.at(mid));
}   // end setMetricSelected


void MetricsDisplayDialog::setRowSelected( int rowid)
{
    int mid = ui->table->item(rowid, ID_COL)->text().toInt();
    emit onSelectMetric(mid);
}   // end setRowSelected


void MetricsDisplayDialog::doOnRowChanged( QTableWidgetItem* m)
{
    MC::Ptr mc = MCM::metric( ui->table->item( m->row(), ID_COL)->text().toInt());
    switch ( m->column())
    {
        case NAME_COL:
            mc->type()->setName( m->text().toStdString());
            break;
        case DESC_COL:
            mc->type()->setDescription( m->text().toStdString());
            break;
        case SHOW_COL:
            mc->setVisible( m->checkState() == Qt::Checked);
            break;
    }   // end switch
    mc->signalUpdated();
}   // end doOnRowChanged


void MetricsDisplayDialog::setShowMetricsAction( QAction* action) { ui->showMetricsButton->setDefaultAction(action);}
void MetricsDisplayDialog::setShowChartAction( QAction* action) { ui->showChartButton->setDefaultAction(action);}



void MetricsDisplayDialog::populate()
{
    populateHPOs( HPOMan::ids());
    populateSyndromes( SynMan::ids());

    // Place all the metrics into the table.
    ui->table->clearContents();
    for ( int mid : MCM::ids())
        appendRow( mid);
    ui->table->resizeColumnsToContents();
    //adjustSize();
}   // end populate


void MetricsDisplayDialog::populateHPOs( const IntSet& hids)
{
    ui->hpoComboBox->clear();
    ui->hpoComboBox->addItem( "Any", -1);
    for ( int hid : hids)
        ui->hpoComboBox->addItem( HPOMan::hpo(hid)->name(), hid);
    ui->hpoComboBox->setEnabled( HPOMan::count() > 0);
}   // end populateHPOs


void MetricsDisplayDialog::populateSyndromes( const IntSet& sids)
{
    ui->syndromesComboBox->clear();
    ui->syndromesComboBox->addItem( "Any", -1);
    for ( int sid : sids)
        ui->syndromesComboBox->addItem( SynMan::syndrome(sid)->name(), sid);
    ui->syndromesComboBox->setEnabled( SynMan::count() > 0);
}   // end populateSyndromes



void MetricsDisplayDialog::appendRow( int mid)
{
    MC::Ptr mc = MCM::metric( mid);

    disconnect( ui->table, &QTableWidget::itemChanged, this, &MetricsDisplayDialog::doOnRowChanged);

    const int rowid = _mIdRows[mid] = ui->table->rowCount();    // Id of new row to be entered
    ui->table->insertRow(rowid);

    // Id (hidden)
    QTableWidgetItem* iitem = new QTableWidgetItem( QString("%1").arg(mid));
    iitem->setFlags( Qt::ItemIsEnabled);
    ui->table->setItem( rowid, ID_COL, iitem);

    // Name
    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    ui->table->setItem( rowid, NAME_COL, nitem);

    // Category
    QTableWidgetItem* citem = new QTableWidgetItem( mc->category());
    citem->setFlags( Qt::ItemIsEnabled);
    ui->table->setItem( rowid, CATG_COL, citem);

    // Description
    QTableWidgetItem* ditem = new QTableWidgetItem( mc->description());
    ditem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    ui->table->setItem( rowid, DESC_COL, ditem);

    // Visibility
    QTableWidgetItem* vitem = new QTableWidgetItem;
    vitem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    vitem->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    ui->table->setItem( rowid, SHOW_COL, vitem);

    connect( ui->table, &QTableWidget::itemChanged, this, &MetricsDisplayDialog::doOnRowChanged);

    ui->table->selectRow( rowid);
}   // end appendRow


void MetricsDisplayDialog::updateRow( int rowid)
{
    MC::Ptr mc = MCM::metric( ui->table->item(rowid,0)->text().toInt());
    ui->table->item( rowid, NAME_COL)->setText( mc->name());
    ui->table->item( rowid, CATG_COL)->setText( mc->category());
    ui->table->item( rowid, DESC_COL)->setText( mc->description());
    ui->table->item( rowid, SHOW_COL)->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    ui->table->resizeColumnsToContents();
}   // end updateRow


// private slot
void MetricsDisplayDialog::doOnUserSelectedHPOTerm()
{
    int hid = ui->hpoComboBox->currentData().toInt();

    IntSet mset;
    if ( hid >= 0)
        mset = HPOMan::hpo(hid)->metrics();
    else    // If -1, then it's all metrics for the HPO terms related to the currently selected syndrome
    {
        int sid = ui->syndromesComboBox->currentData().toInt();
        const IntSet* hset = &SynMan::syndrome(sid)->hpos(); // Get the HPO ids related to the current syndrome
        for ( int h : *hset)
        {
            const IntSet& mset0 = HPOMan::hpo(h)->metrics();
            mset.insert( mset0.begin(), mset0.end());
        }   // end for
    }   // end else

    // Show/hide the metric table rows according to if the name of the metric is in the set for the selected HPO term.
    int rowCount = ui->table->rowCount();
    for ( int i = 0; i < rowCount; ++i)
    {
        int mid = ui->table->item(i, ID_COL)->text().toInt();
        MC::Ptr mc = MCM::metric( mid);
        const bool wasVisible = mc->isVisible();

        if ( mset.count(mid) > 0)
        {
            ui->table->showRow( i);
            mc->setVisible( ui->table->item(i, SHOW_COL)->checkState() == Qt::Checked);
        }   // end if
        else
        {
            ui->table->hideRow( i);
            mc->setVisible(false);
        }   // end else

        if ( mc->isVisible() != wasVisible)
            mc->signalUpdated();
    }   // end for
}   // end doOnUserSelectedHPOTerm


void MetricsDisplayDialog::doOnUserSelectedSyndrome()
{
    int sid = ui->syndromesComboBox->currentData().toInt();
    const IntSet *hset = sid >= 0 ? &SynMan::syndrome(sid)->hpos() :  &HPOMan::ids();
    populateHPOs(*hset);
    doOnUserSelectedHPOTerm();
}   // end doOnUserSelectedSyndrome
