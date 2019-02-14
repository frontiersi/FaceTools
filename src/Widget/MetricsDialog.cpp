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

#include <MetricsDialog.h>
#include <CheckAllTableHeader.h>
#include <ui_MetricsDialog.h>
#include <IntTableWidgetItem.h>
#include <MetricCalculatorManager.h>
#include <SyndromeManager.h>
#include <ModelSelector.h>
#include <PhenotypeManager.h>
#include <FaceModel.h>
#include <FileIO.h>
#include <algorithm>
#include <QHeaderView>
#include <QDoubleSpinBox>
#include <QToolTip>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::MetricsDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using SynMan = FaceTools::Metric::SyndromeManager;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MC = FaceTools::Metric::MetricCalculator;
using FaceTools::Action::ModelSelector;
using FaceTools::Vis::FV;
using FaceTools::FM;

namespace {
enum ColIndex
{
    SHOW_COL = 0,
    IDNT_COL = 1,
    NAME_COL = 2,
    CATG_COL = 3,
    DESC_COL = 4
};  // end enum
}   // end namespace


MetricsDialog::MetricsDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::MetricsDialog), _syndromeToPhenotype(false)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Measurements");
    resize( 850, 400);

    _ui->table->setColumnCount(5);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name", "Category", "Description"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table);
    _ui->table->setHorizontalHeader(header);

    connect( header, &CheckAllTableHeader::allChecked, this, &MetricsDialog::doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &MetricsDialog::sortOnColumn);

    //_ui->table->setSelectionBehavior( QAbstractItemView::SelectRows);
    //_ui->table->setSelectionMode( QAbstractItemView::SingleSelection);
    //_ui->table->setSelectionMode( QAbstractItemView::NoSelection);
    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::StrongFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    header->setSortIndicatorShown(false);

    // Leveraging signal activated since this is only emitted as a result of user interaction.
    connect( _ui->syndromesComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::doOnUserSelectedSyndrome);
    connect( _ui->hpoComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::doOnUserSelectedHPOTerm);

    connect( _ui->showChartButton, &QToolButton::clicked, this, &MetricsDialog::onShowChart);
    connect( _ui->showPhenotypesButton, &QToolButton::clicked, this, &MetricsDialog::onShowPhenotypes);
    connect( _ui->flipButton, &QToolButton::clicked, this, &MetricsDialog::doOnClickedFlipCombosButton);

    connect( _ui->matchedCheckBox, &QCheckBox::clicked, this, &MetricsDialog::doOnClickedMatchButton);
    connect( _ui->ignoreEthnicityCheckBox, &QCheckBox::clicked, this, &MetricsDialog::doOnEthnicityIgnored);

    //_ui->table->setColumnHidden(IDNT_COL, true);
    header->setStretchLastSection(true);   // Resize width of final column

    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::selected, this, &MetricsDialog::doOnSetCurrentMetric);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    populate();

    connect( _ui->table, &QTableWidget::cellClicked, [this]( int rw){ setSelectedRow(rw);});
    connect( _ui->table, &QTableWidget::currentItemChanged, [this]( QTableWidgetItem* item){ setSelectedRow(item->row());});
    //connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);
}   // end ctor


MetricsDialog::~MetricsDialog() { delete _ui;}


void MetricsDialog::setShowScanInfoAction( QAction* a)
{
    _ui->showScanInfoButton->setDefaultAction(a);
}   // end setShowScanInfoAction


void MetricsDialog::setShowMetricsAction( QAction* a)
{
    _ui->showMetricsButton->setDefaultAction(a);
}   // end setShowMetricsAction


void MetricsDialog::setDetectedOnlyEnabled( bool v)
{
    _ui->matchedCheckBox->setEnabled(v);
}   // end setDetectedOnlyEnabled


void MetricsDialog::doOnSetAllChecked( bool c)
{
    const Qt::CheckState cstate = c ? Qt::Checked : Qt::Unchecked;

    disconnect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);

    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        if ( !_ui->table->isRowHidden(i))   // Only affects visible rows
        {
            _ui->table->item( i, SHOW_COL)->setCheckState( cstate);
            MC::Ptr mc = MCM::metric( _ui->table->item( i, IDNT_COL)->text().toInt());
            mc->setVisible(c);
        }   // end if
    }   // end for

    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);

    emit onRefreshedMetrics();
}   // end doOnSetAllChecked


void MetricsDialog::sortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    resetIdRowMap();
}   // end sortOnColumn


void MetricsDialog::resetIdRowMap()
{
    _idRows.clear();    // Re-map metric IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
        _idRows[mid] = i;
    }   // end for
}   // end resetIdRowMap


void MetricsDialog::doOnSetCurrentMetric( int mid)
{
    assert( _idRows.count(mid) > 0);
    highlightRow( _idRows.at(mid));
}   // end doOnSetCurrentMetric


void MetricsDialog::highlightRow( int rowid)
{
    disconnect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);

    MC::Ptr pam = MCM::previousMetric();
    if ( pam)
    {
        const int prowid = _idRows.at( pam->id());
        _ui->table->item( prowid, SHOW_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, IDNT_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, NAME_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, CATG_COL)->setBackgroundColor( Qt::white);
        _ui->table->item( prowid, DESC_COL)->setBackgroundColor( Qt::white);
    }   // end if

    static const QColor bg(200,235,255);
    _ui->table->item( rowid, SHOW_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, IDNT_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, NAME_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, CATG_COL)->setBackgroundColor( bg);
    _ui->table->item( rowid, DESC_COL)->setBackgroundColor( bg);

    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::doOnItemChanged);
}   // end highlightRow


void MetricsDialog::setSelectedRow( int rowid)
{
    const int mid = _ui->table->item( rowid, IDNT_COL)->text().toInt();
    MCM::setCurrentMetric( mid); // Will cause the metric to become active which will in turn call doOnSetMetricActive
}   // end setSelectedRow


void MetricsDialog::doOnItemChanged( QTableWidgetItem* m)
{
    MC::Ptr mc = MCM::metric( _ui->table->item( m->row(), IDNT_COL)->text().toInt());
    switch ( m->column())
    {
        case SHOW_COL:
            mc->setVisible( m->checkState() == Qt::Checked && !_ui->table->isRowHidden( m->row()));
            disconnect( &*mc, &MC::selected, this, &MetricsDialog::doOnSetCurrentMetric);
            qInfo() << "Set selected metric " << mc->id();
            mc->setSelected();
            connect( &*mc, &MC::selected, this, &MetricsDialog::doOnSetCurrentMetric);
            break;
            /*
        case NAME_COL:
            mc->setName( m->text());
            break;
        case DESC_COL:
            mc->setDescription( m->text());
            break;
            */
    }   // end switch
}   // end doOnItemChanged


void MetricsDialog::populate()
{
    populateHPOs( HPOMan::ids());
    populateSyndromes( SynMan::ids());

    // Place all the metrics into the table.
    _ui->table->clearContents();
    for ( int mid : MCM::ids())
        appendRow( mid);

    sortOnColumn( NAME_COL);
    setSelectedRow( 0);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();

    CheckAllTableHeader* header = qobject_cast<CheckAllTableHeader*>(_ui->table->horizontalHeader());
    header->setAllChecked(true);

    doOnClickedFlipCombosButton();
}   // end populate


void MetricsDialog::populateHPOs( const IntSet& hids)
{
    _ui->hpoComboBox->clear();
    _ui->hpoComboBox->addItem( "-- Any --", -1);

    std::unordered_map<QString, int> nids;
    QStringList slst;
    for ( int hid : hids)
    {
        QString nm = HPOMan::phenotype(hid)->name();
        slst.append(nm);
        nids[nm] = hid;
    }   // end for

    slst.sort();
    for ( const QString& nm : slst)
        _ui->hpoComboBox->addItem( nm, nids.at(nm));

    _ui->hpoComboBox->setEnabled( _ui->hpoComboBox->count() > 1);
}   // end populateHPOs


void MetricsDialog::populateSyndromes( const IntSet& sids)
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



void MetricsDialog::appendRow( int mid)
{
    MC::Ptr mc = MCM::metric( mid);
    assert(mc);

    const int rowid = _idRows[mid] = _ui->table->rowCount();    // Id of new row to be entered
    _ui->table->insertRow(rowid);

    QTableWidgetItem* iitem = new IntTableWidgetItem( mid, 5);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, IDNT_COL, iitem);

    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    QTableWidgetItem* citem = new QTableWidgetItem( mc->category());
    citem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, CATG_COL, citem);

    QTableWidgetItem* ditem = new QTableWidgetItem( mc->description());
    ditem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, DESC_COL, ditem);

    QTableWidgetItem* vitem = new QTableWidgetItem;
    vitem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    vitem->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->setItem( rowid, SHOW_COL, vitem);
}   // end appendRow


/*
void MetricsDialog::updateRow( int rowid)
{
    MC::Ptr mc = MCM::metric( _ui->table->item(rowid,IDNT_COL)->text().toInt());
    _ui->table->item( rowid, SHOW_COL)->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->item( rowid, NAME_COL)->setText( mc->name());
    _ui->table->item( rowid, CATG_COL)->setText( mc->category());
    _ui->table->item( rowid, DESC_COL)->setText( mc->description());
    _ui->table->resizeColumnsToContents();
}   // end updateRow
*/


// private slot
void MetricsDialog::doOnUserSelectedHPOTerm()
{
    if ( _ui->hpoComboBox->count() == 2)
        _ui->hpoComboBox->setCurrentIndex(1);
    const int hid = _ui->hpoComboBox->currentData().toInt();
    const int sid = _ui->syndromesComboBox->currentData().toInt();

    IntSet smset;   // Fill with selected metrics if necessary
    const IntSet *mset = &smset;

    if ( hid >= 0)
        mset = &HPOMan::phenotype(hid)->metrics(); // Metrics just from the selected HPO term
    else if ( sid < 0)   // Any syndrome too, so use all metrics in the matched set of phenotypes
    {
        for ( int h : _mpids)
        {
            const IntSet& mset0 = HPOMan::phenotype(h)->metrics();
            smset.insert( mset0.begin(), mset0.end());
        }   // end for
    }   // end else if
    else    // If -1, then it's all metrics for the HPO terms related to the currently selected syndrome
    {
        for ( int h : SynMan::syndrome(sid)->hpos()) // Get the HPO ids related to the currently selected syndrome
        {
            if ( _mpids.count(h) > 0)   // Restrict to only those
            {
                const IntSet& mset0 = HPOMan::phenotype(h)->metrics();
                smset.insert( mset0.begin(), mset0.end());
            }   // end if
        }   // end for
    }   // end else

    // Show/hide the metric table rows according to if the name of the metric is in the set for the selected HPO term.
    int rowCount = _ui->table->rowCount();
    int smid = -1;
    MC::Ptr pam = MCM::currentMetric();

    for ( int i = 0; i < rowCount; ++i)
    {
        int mid = _ui->table->item(i, IDNT_COL)->text().toInt();
        MC::Ptr mc = MCM::metric( mid);

        if ( mset->count(mid) > 0)
        {
            _ui->table->showRow( i);
            mc->setVisible( _ui->table->item(i, SHOW_COL)->checkState() == Qt::Checked);
            if ( smid < 0 || mc == pam)
                smid = mid;
        }   // end if
        else
        {
            _ui->table->hideRow( i);
            mc->setVisible(false);
        }   // end else
    }   // end for

    if ( !_syndromeToPhenotype)
    { // Re-populate the syndromes combo box with only those related to the selected phenotype
        if ( hid >= 0)
            populateSyndromes( SynMan::hpoSyndromes(hid));
        else
            populateSyndromes( SynMan::ids());  // Repopulate with all syndromes
    }   // end if

    if ( smid >= 0)
        setSelectedRow( _idRows.at(smid));

    emit onRefreshedMetrics();
}   // end doOnUserSelectedHPOTerm


void MetricsDialog::doOnUserSelectedSyndrome()
{
    if ( _syndromeToPhenotype)
    {
        if ( _ui->syndromesComboBox->count() == 2)
            _ui->syndromesComboBox->setCurrentIndex(1); // Not 'any' if only two syndromes

        if ( _ui->syndromesComboBox->currentData().toInt() < 0) // Any syndrome
            _ui->hpoComboBox->setCurrentIndex( 0);  // So any phenotype

        refresh();
    }   // end if
}   // end doOnUserSelectedSyndrome


// public
void MetricsDialog::refresh()
{
    const bool ignoreEthnicity = _ui->ignoreEthnicityCheckBox->isChecked();

    int csid = _ui->syndromesComboBox->currentData().toInt();
    int cpid = _ui->hpoComboBox->currentData().toInt();

    const IntSet* hset = nullptr;
    if ( _syndromeToPhenotype && csid >= 0)
        hset = &SynMan::syndrome(csid)->hpos();
    else
        hset = &HPOMan::ids();

    _mpids.clear();

    // Restrict the current set of phenotypes to only those matched on the currently selected model
    if ( _ui->matchedCheckBox->isChecked())
    {
        assert( ModelSelector::selected());
        FM* fm = ModelSelector::selected()->data();
        for ( int hid : *hset)
        {
            if ( fm->phenotypes().count(hid) > 0)
            {
                if ( ignoreEthnicity || HPOMan::phenotype(hid)->isDemographicMatch( fm))
                    _mpids.insert(hid);
            }   // end if
        }   // end for
    }   // end if
    else
        _mpids = *hset; // Straight copy

    if ( _mpids.count(cpid) == 0)
        cpid = -1;

    populateHPOs( _mpids);

    // Tell others of matches to phenotypes
    emit onShowingPhenotypes( _mpids);

    int rowid = cpid >= 0 ? _ui->hpoComboBox->findData(cpid) : 0;
    _ui->hpoComboBox->setCurrentIndex( rowid);
    doOnUserSelectedHPOTerm();

    if ( !_syndromeToPhenotype)
    {
        if ( cpid >= 0)
            populateSyndromes( SynMan::hpoSyndromes(cpid));
        else
            populateSyndromes( IntSet());   // Empty set
    }   // end if
}   // end refresh


void MetricsDialog::doOnClickedMatchButton()
{
    if ( !_ui->matchedCheckBox->isChecked())
        _ui->hpoComboBox->setCurrentIndex(0);   // Any phenotype
    refresh();
}   // end doOnClickedMatchButton


void MetricsDialog::doOnEthnicityIgnored()
{
    emit onEthnicityIgnored(_ui->ignoreEthnicityCheckBox->isChecked());
    refresh();
}   // end doOnEthnicityIgnored


void MetricsDialog::doOnClickedFlipCombosButton()
{
    _syndromeToPhenotype = !_syndromeToPhenotype;
    QString iconName = ":/icons/ABOVE";
    QString toolTipText = tr("Restrict the list of syndromes to the currently selected phenotype.");
    if ( _syndromeToPhenotype)
    {
        iconName= ":/icons/BELOW";
        toolTipText = tr("Restrict the list of phenotypes to the currently selected syndrome.");
    }   // end if
    _ui->flipButton->setIcon( QIcon(iconName));
    _ui->flipButton->setToolTip( toolTipText);

    if ( _syndromeToPhenotype)
    {
        QVariant v = _ui->syndromesComboBox->currentData();
        populateSyndromes( SynMan::ids());
        _ui->syndromesComboBox->setCurrentIndex( _ui->syndromesComboBox->findData( v));
        doOnUserSelectedSyndrome();
    }   // eif
    else
    {
        QVariant v = _ui->hpoComboBox->currentData();
        populateHPOs( HPOMan::ids());
        _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData(v));
        doOnUserSelectedHPOTerm();
    }   // end else

    if ( _ui->matchedCheckBox->isChecked())
        refresh();

    // Show the tooltip on click.
    QToolTip::showText( _ui->flipButton->mapToGlobal(QPoint()), toolTipText);
}   // end doOnClickedFlipCombosButton
