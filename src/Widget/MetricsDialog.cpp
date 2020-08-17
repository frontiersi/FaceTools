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

#include <Widget/MetricsDialog.h>
#include <ui_MetricsDialog.h>

#include <Widget/CheckAllTableHeader.h>
#include <Widget/IntTableWidgetItem.h>

#include <Metric/MetricManager.h>
#include <Metric/SyndromeManager.h>
#include <Metric/PhenotypeManager.h>

#include <Ethnicities.h>
#include <FaceModel.h>
#include <Action/ActionUpdateGrowthData.h>
#include <Action/ModelSelector.h>

#include <QHeaderView>
#include <QToolTip>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::MetricsDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using SynMan = FaceTools::Metric::SyndromeManager;
using MM = FaceTools::Metric::MetricManager;
using MC = FaceTools::Metric::Metric;
using MS = FaceTools::Action::ModelSelector;
using GDS = FaceTools::Metric::GrowthDataSources;
using GD = FaceTools::Metric::GrowthData;

namespace {
enum ColIndex
{
    SHOW_COL = 0,
    IDNT_COL = 1,
    NAME_COL = 2,
    DESC_COL = 3
};  // end enum
}   // end namespace


MetricsDialog::MetricsDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::MetricsDialog), _syndromeToPhenotype(true), _prowid(-1)
{
    _ui->setupUi(this);
    _ui->splitter->setStretchFactor( 0, 6);
    _ui->splitter->setStretchFactor( 1, 1);
    _ui->remarksTextBrowser->setOpenExternalLinks(true);

    setWindowTitle( parent->windowTitle() + " | Measurements Browser");

    //setWindowFlags( windowFlags() & ~Qt::WindowCloseButtonHint);
    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->table->setColumnCount(4);
    _ui->table->setHorizontalHeaderLabels( QStringList( {"", "ID", "Name", "Description"}));

    CheckAllTableHeader* header = new CheckAllTableHeader( _ui->table, true/* use eye icon */);
    _ui->table->setHorizontalHeader(header);

    connect( header, &CheckAllTableHeader::allChecked, this, &MetricsDialog::_doOnSetAllChecked);
    connect( header, &CheckAllTableHeader::sectionClicked, this, &MetricsDialog::_doSortOnColumn);

    //_ui->table->setSelectionBehavior( QAbstractItemView::SelectRows);
    //_ui->table->setSelectionMode( QAbstractItemView::SingleSelection);
    //_ui->table->setSelectionMode( QAbstractItemView::NoSelection);
    _ui->table->setShowGrid(false);
    _ui->table->setFocusPolicy( Qt::StrongFocus);
    _ui->table->setEditTriggers( QAbstractItemView::NoEditTriggers);
    _ui->table->setSortingEnabled(true);
    header->setSortIndicatorShown(false);

    // Leveraging signal activated since this is only emitted as a result of user interaction.
    //connect( _ui->flipButton, &QToolButton::clicked, [this](){ _doOnClickedFlipCombosButton();});
    _ui->flipButton->setVisible(false);

#ifdef NDEBUG
    _ui->table->setColumnHidden(IDNT_COL, true);    // Hide the metric ID column in release mode
#endif
    header->setStretchLastSection(true);   // Resize width of final column
    header->setMinimumSectionSize(14);

    _populateSyndromes();
    _refreshPhenotypes( HPOMan::ids());

    _populateRegionType();
    _populateMetricType();
    _populateTable();

    connect( _ui->typeComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedType);
    connect( _ui->regionComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedRegion);
    connect( _ui->synComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedSyndrome);
    connect( _ui->matchAtypicalToolButton, &QToolButton::toggled, this, &MetricsDialog::_doOnClickedAtypical);

    connect( _ui->hpoComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedPhenotype);
    connect( _ui->autoStatsCheckBox, &QCheckBox::clicked, this, &MetricsDialog::_doOnClickedAutoStats);

    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectSex);
    connect( _ui->ethnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectEthnicity);
    connect( _ui->sourceComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectSource);
    connect( _ui->inPlaneCheckBox, &QCheckBox::clicked, this, &MetricsDialog::_doOnClickedForceInPlane);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _ui->table->setColumnWidth( SHOW_COL, 18);

    connect( _ui->table, &QTableWidget::currentItemChanged, this, &MetricsDialog::_doOnChangeTableRow);
    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);
    connect( _ui->showChartButton, &QToolButton::clicked, this, &MetricsDialog::onShowChart);

    _doOnChangeTableRow( _ui->table->item(0,0));
}   // end ctor


MetricsDialog::~MetricsDialog() { delete _ui;}


void MetricsDialog::setShowScanInfoAction( QAction *act)
{
    _ui->showScanInfoButton->setDefaultAction( act);
    connect( act, &QAction::toggled, _ui->showScanInfoButton, &QAbstractButton::setChecked);
}   // end setShowScanInfoAction


void MetricsDialog::setShowPhenotypesAction( QAction *act)
{
    _ui->showPhenotypesButton->setDefaultAction( act);
    //connect( act, &QAction::toggled, _ui->showPhenotypesButton, &QAbstractButton::setChecked);
    _ui->showPhenotypesButton->setCheckable(true);
    connect( act, &QAction::toggled, [this]( bool v){ _ui->showPhenotypesButton->setChecked(v);});
}   // end setShowPhenotypesAction


void MetricsDialog::closeEvent( QCloseEvent* e)
{
    e->accept();
    accept();
}   // end closeEvent


void MetricsDialog::_doOnSetAllChecked( bool c)
{
    const Qt::CheckState cstate = c ? Qt::Checked : Qt::Unchecked;
    _ui->table->blockSignals(true);

    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        if ( !_ui->table->isRowHidden(i))   // Only affects visible rows
        {
            _ui->table->item( i, SHOW_COL)->setCheckState( cstate);
            const int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
            MM::metric(mid)->setVisible(c);
        }   // end if
    }   // end for

    _ui->table->blockSignals(false);
    emit onRefreshAllMetrics();
}   // end _doOnSetAllChecked


void MetricsDialog::_doSortOnColumn( int cidx)
{
    _ui->table->sortItems(cidx);    // Sort into ascending order
    _idRows.clear();    // Re-map metric IDs to row indices
    const int nrows = _ui->table->rowCount();
    for ( int i = 0; i < nrows; ++i)
    {
        int mid = _ui->table->item( i, IDNT_COL)->text().toInt();
        _idRows[mid] = i;
    }   // end for
}   // end _doSortOnColumn


void MetricsDialog::_doOnChangeTableRow( QTableWidgetItem *item)
{
    _setCurrentMetric( _ui->table->item( item->row(), IDNT_COL)->text().toInt());
}   // end _doOnChangeTableRow


void MetricsDialog::_doOnItemChanged( QTableWidgetItem* m)
{
    bool newVis = false;
    const int mid = _ui->table->item( m->row(), IDNT_COL)->text().toInt();
    switch ( m->column())
    {
        case SHOW_COL:
            newVis = m->checkState() == Qt::Checked && !_ui->table->isRowHidden( m->row());
            MM::metric(mid)->setVisible( newVis);
            break;
    }   // end switch
    _setCurrentMetric( mid);
}   // end _doOnItemChanged


void MetricsDialog::_populateTable()
{
    // Place all the metrics into the table.
    _ui->table->clearContents();
    for ( int mid : MM::ids())
        _appendRow( mid);

    _doSortOnColumn( NAME_COL);
    _ui->table->resizeColumnsToContents();
    _ui->table->resizeRowsToContents();
    _ui->table->scrollToTop();

    CheckAllTableHeader* header = qobject_cast<CheckAllTableHeader*>(_ui->table->horizontalHeader());
    header->setAllChecked(true);
}   // end _populateTable


void MetricsDialog::_populateRegionType()
{
    _ui->regionComboBox->clear();
    _ui->regionComboBox->addItem( ANY_ITEM, -1);

    QSet<QString> rset;
    for ( int mid : MM::ids())
        rset.insert( MM::metric(mid)->region());
    QStringList rlst( rset.begin(), rset.end());
    rlst.sort();

    _ui->regionComboBox->addItems( rlst);
    _ui->regionComboBox->setEnabled( !rlst.empty());
    _ui->regionComboBox->setCurrentIndex( 0);
}   // end _populateRegionType


void MetricsDialog::_populateMetricType()
{
    _ui->typeComboBox->clear();
    _ui->typeComboBox->addItem( ANY_ITEM, -1);

    QSet<QString> tset;
    for ( int mid : MM::ids())
        tset.insert( MM::metric(mid)->category());
    QStringList tlst( tset.begin(), tset.end());
    tlst.sort();

    _ui->typeComboBox->addItems( tlst);
    _ui->typeComboBox->setEnabled( !tlst.empty());
    _ui->typeComboBox->setCurrentIndex( 0);
}   // _populateMetricType


void MetricsDialog::_refreshPhenotypes( const IntSet &hids)
{
    int hid = -1;
    if ( _ui->hpoComboBox->count() > 0)
    {
        hid = _ui->hpoComboBox->currentData().toInt();
        if ( hids.count(hid) == 0)
            hid = -1;
    }   // end if

    _ui->hpoComboBox->clear();
    _ui->hpoComboBox->addItem( ANY_ITEM, -1);

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

    _ui->hpoComboBox->setEnabled( !slst.empty());
    _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData( hid));
    emit onUpdateMatchingPhenotypes( hids);
    if ( hid != -1)
        emit onSelectedHPOTerm( hid);
}   // end _refreshPhenotypes


void MetricsDialog::_populateSyndromes()
{
    _ui->synComboBox->clear();
    _ui->synComboBox->addItem( ANY_ITEM, -1);

    const IntSet *sids = &SynMan::ids();
    if ( !_syndromeToPhenotype)
    {
        const int hid = _ui->hpoComboBox->currentData().toInt();
        if ( hid >= 0)
            sids = &SynMan::hpoSyndromes(hid);
    }   // end if

    std::unordered_map<QString, int> nids;
    QStringList slst;
    for ( int sid : *sids)
    {
        QString nm = SynMan::syndrome(sid)->name();
        slst.append(nm);
        nids[nm] = sid;
    }   // end for

    slst.sort();
    for ( const QString& nm : slst)
        _ui->synComboBox->addItem( nm, nids.at(nm));

    //_ui->synComboBox->setEnabled( !slst.empty());
    _ui->synComboBox->setEnabled( false);   // ENABLE LATER
    _ui->synComboBox->setCurrentIndex( 0);
}   // end _populateSyndromes


void MetricsDialog::_appendRow( int mid)
{
    Metric::Metric::Ptr mc = MM::metric( mid);
    assert(mc);

    const int rowid = _idRows[mid] = _ui->table->rowCount();    // Id of new row to be entered
    _ui->table->insertRow(rowid);

    QTableWidgetItem* iitem = new IntTableWidgetItem( mid, 5);
    iitem->setFlags( Qt::ItemIsEnabled);
    _ui->table->setItem( rowid, IDNT_COL, iitem);

    QTableWidgetItem* nitem = new QTableWidgetItem( mc->name());
    nitem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, NAME_COL, nitem);

    QTableWidgetItem* ditem = new QTableWidgetItem( mc->description());
    ditem->setFlags( Qt::ItemIsEnabled);// | Qt::ItemIsEditable);
    _ui->table->setItem( rowid, DESC_COL, ditem);

    QTableWidgetItem* vitem = new QTableWidgetItem;
    vitem->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
    vitem->setCheckState( mc->isVisible() ? Qt::Checked : Qt::Unchecked);
    _ui->table->setItem( rowid, SHOW_COL, vitem);
}   // end _appendRow


namespace {

IntSet filterMetrics( const IntSet &mmids, const QString &mtype, const QString &region)
{
    IntSet mids;
    for ( int mid : mmids)
    {
        MC::CPtr mc = MM::metric(mid);
        if ( (region.isEmpty() || mc->region() == region) && (mtype.isEmpty() || mc->category() == mtype))
            mids.insert(mid);
    }   // end for
    return mids;
}   // end filterMetrics


IntSet filterHPOTermsByMetrics( const IntSet &hids0, const IntSet &mids)
{
    IntSet hids1;
    for ( int hid : hids0)
    {
        for ( int mid : HPOMan::phenotype(hid)->metrics()) // For each metric associated with this HPO term
        {
            if ( mids.count(mid) > 0)
            {
                hids1.insert(hid);
                break;
            }   // end if
        }   // end for
    }   // end for
    return hids1;
}   // end filterHPOTermsByMetrics


IntSet filterHPOTermsByModelMatched( const IntSet &hids)
{
    const FaceTools::FM *fm = MS::selectedModel();
    IntSet hids1;
    for ( int hid : hids)
        if ( !fm || HPOMan::phenotype(hid)->isPresent(fm, -1))
            hids1.insert(hid);
    return hids1;
}   // end filterHPOTermsByModelMatched


IntSet filterMetricsByHPOTerms( const IntSet &mids, const IntSet &hids)
{
    IntSet mids1;
    for ( int mid : mids)
    {
        for ( int hid : HPOMan::byMetric(mid))
        {
            if ( hids.count(hid) > 0)
            {
                mids1.insert(mid);
                break;
            }   // end if
        }   // end for
    }   // end for
    return mids1;
}   // end filterMetricsByHPOTerms

}   // end namespace


IntSet MetricsDialog::_getModelMatchedPhenotypes( const IntSet &mids) const
{
    const int sid = _ui->synComboBox->currentData().toInt(); // Get HPO terms that match the selected syndrome
    const IntSet &hids = sid < 0 ? HPOMan::ids() : SynMan::syndrome(sid)->hpos();
    // Filter HPO terms further by matched metrics (only those with at least one associated metric also in mids).
    IntSet ohids = filterHPOTermsByMetrics( hids, mids);
    // Further filter HPO terms by only those that match the current model (if option checked)
    if ( _ui->matchAtypicalToolButton->isChecked())
        ohids = filterHPOTermsByModelMatched( ohids);
    return ohids;
}   // end _getModelMatchedPhenotypes


IntSet MetricsDialog::_getModelMatchedMetrics( int hid) const
{
    const QString mtype = _ui->typeComboBox->currentIndex() > 0 ? _ui->typeComboBox->currentText() : "";
    const QString region = _ui->regionComboBox->currentIndex() > 0 ? _ui->regionComboBox->currentText() : "";
    IntSet mids = filterMetrics( hid >= 0 ? HPOMan::phenotype(hid)->metrics() : MM::ids(), mtype, region);
    if ( _ui->matchAtypicalToolButton->isChecked())
    {
        const IntSet hids = filterHPOTermsByModelMatched( HPOMan::ids());
        mids = filterMetricsByHPOTerms( mids, hids);
    }   // end if
    return mids;
}   // end _getModelMatchedMetrics


int MetricsDialog::_refreshDisplayedRows( const IntSet &mids)
{
    int smid = -1;
    int pam = -1;
    if ( MM::currentMetric())
        pam = MM::currentMetric()->id();

    for ( int i = 0; i < _ui->table->rowCount(); ++i)
    {
        const int mid = _ui->table->item(i, IDNT_COL)->text().toInt();
        MC::Ptr mc = MM::metric(mid);

        if ( mids.count(mid) > 0)  // Should show this one
        {
            if ( smid < 0 || mid == pam)
                smid = mid;
            if ( _ui->table->isRowHidden(i))
            {
                _ui->table->showRow( i);
                mc->setVisible(true);
            }   // end if
        }   // end if
        else if ( !_ui->table->isRowHidden(i))
        {
            _ui->table->hideRow( i);
            mc->setVisible(false);
        }   // end else if
    }   // end for

    return smid;
}   // end _refreshDisplayedRows


void MetricsDialog::_refreshAvailableSexesFromMetric()
{
    _ui->sexComboBox->clear();
    const MC::Ptr mc = MM::currentMetric();
    if ( mc)
    {
        const std::unordered_set<int8_t> sexs = mc->growthData().sexes();
        assert( sexs.size() <= 3);
        if ( sexs.count(UNKNOWN_SEX) > 0)
            _ui->sexComboBox->addItem( toLongSexString(UNKNOWN_SEX), UNKNOWN_SEX);
        if ( sexs.count(FEMALE_SEX) > 0)
            _ui->sexComboBox->addItem( toLongSexString(FEMALE_SEX), FEMALE_SEX);
        if ( sexs.count(MALE_SEX) > 0)
            _ui->sexComboBox->addItem( toLongSexString(MALE_SEX), MALE_SEX);
    }   // end if
}   // end _refreshAvailableSexesFromMetric


void MetricsDialog::_refreshAvailableEthnicitiesFromMetric()
{
    _ui->ethnicityComboBox->clear();
    const MC::Ptr mc = MM::currentMetric();
    if ( mc)
    {
        const IntSet eset = mc->growthData().ethnicities();
        QStringList elst;
        std::unordered_map<QString, int> emap;
        for ( int e : eset)
        {
            const QString& en = Ethnicities::name(e);
            if ( emap.count(en) == 0)
            {
                emap[en] = e;
                elst << en;
            }   // end if
        }   // end for
        elst.sort();    // Sort ethnicities
        for ( const QString& en : elst)
            _ui->ethnicityComboBox->addItem( en, emap[en]);
    }   // end if
}   // end _refreshAvailableEthnicitiesFromMetric


void MetricsDialog::_highlightRow( int mid)
{
    const int rowid = mid >= 0 ? _idRows.at(mid) : -1;
    _ui->table->blockSignals(true);

    if ( _prowid >= 0)
    {
        static const QBrush wbrush( Qt::white);
        _ui->table->item( _prowid, SHOW_COL)->setBackground( wbrush);
        _ui->table->item( _prowid, IDNT_COL)->setBackground( wbrush);
        _ui->table->item( _prowid, NAME_COL)->setBackground( wbrush);
        _ui->table->item( _prowid, DESC_COL)->setBackground( wbrush);
    }   // end if

    if ( rowid >= 0)
    {
        static const QBrush BG( QColor(200,235,255));
        _ui->table->item( rowid, SHOW_COL)->setBackground( BG);
        _ui->table->item( rowid, IDNT_COL)->setBackground( BG);
        _ui->table->item( rowid, NAME_COL)->setBackground( BG);
        _ui->table->item( rowid, DESC_COL)->setBackground( BG);
        _ui->table->setCurrentCell( rowid, SHOW_COL);
    }   // end if

    _prowid = rowid;
    _ui->table->blockSignals(false);
}   // end _highlightRow


void MetricsDialog::_setCurrentMetric( int mid)
{
    _highlightRow( mid);
    QString rmks;
    MC::Ptr mc = MM::setCurrentMetric( mid);
    if ( mc)
    {
        rmks = mc->remarks();   // If no specific remarks, set the generic type remarks.
        if ( rmks.isEmpty())
            rmks = mc->typeRemarks();
    }   // end if
    _ui->remarksTextBrowser->setHtml( rmks);
    _refreshAvailableSexesFromMetric();
    _refreshAvailableEthnicitiesFromMetric();
    reflectCurrentMetricStats();
    emit onStatsChanged();
}   // end _setCurrentMetric


void MetricsDialog::show()
{
    QWidget::show();
    raise();
    activateWindow();
    _doOnClickedPhenotype();
}   // end show


void MetricsDialog::_doOnClickedAutoStats()
{
    const bool isAutoStats = _ui->autoStatsCheckBox->isChecked();
    Action::ActionUpdateGrowthData::setAutoStats( isAutoStats);
    if ( isAutoStats)
    {
        _ui->inPlaneCheckBox->setChecked(false);
        MM::setInPlane( false);
    }   // end if
    reflectCurrentMetricStats();
    _doOnClickedRegion();
}   // end _doOnClickedAutoStats


namespace {
void setTestItemEnabled( QStandardItem *item, int8_t sex, int ethn)
{
    if ( item)
    {
        if ( MM::currentMetric()->growthData().hasData( sex, ethn))
            item->setFlags( item->flags() | Qt::ItemIsEnabled);
        else
            item->setFlags( item->flags() & ~Qt::ItemIsEnabled);
    }   // end if
}   // end setTestItemEnabled
}   // end namespace


void MetricsDialog::_doOnSelectEthnicity()
{
    assert( !_ui->autoStatsCheckBox->isChecked());
    int8_t sex = int8_t( _ui->sexComboBox->currentData().toInt());
    const int ethn = _ui->ethnicityComboBox->currentData().toInt();

    if ( !MM::currentMetric()->growthData().hasData( sex, ethn))
    {
        QSignalBlocker blocker(_ui->sexComboBox);
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( UNKNOWN_SEX));
        sex = UNKNOWN_SEX;
    }   // end if

    /*
    QStandardItemModel *cbmodel = qobject_cast<QStandardItemModel*>( _ui->sexComboBox->model());
    setTestItemEnabled( cbmodel->item(1), FEMALE_SEX, ethn);
    setTestItemEnabled( cbmodel->item(2), MALE_SEX, ethn);
    */

    _onSelectSexAndEthnicity( sex, ethn);
}   // end _doOnSelectEthnicity


void MetricsDialog::_doOnSelectSex()
{
    _doOnSelectEthnicity();
    /*
    assert( !_ui->autoStatsCheckBox->isChecked());
    const int8_t sex = int8_t( _ui->sexComboBox->currentData().toInt());
    const int ethn = _ui->ethnicityComboBox->currentData().toInt();
    _onSelectSexAndEthnicity( sex, ethn);
    */
}   // end _doOnSelectSex


void MetricsDialog::_onSelectSexAndEthnicity( int8_t sex, int ethn)
{
    assert( MM::currentMetric());
    const GD *gd = _updateSourcesDropdown( sex, ethn);
    MM::currentMetric()->growthData().setCurrent( gd);
    _doOnClickedRegion();
}   // end _onSelectSexAndEthnicity


void MetricsDialog::_doOnSelectSource()
{
    assert( !_ui->autoStatsCheckBox->isChecked());
    assert( MM::currentMetric());
    Metric::GrowthDataRanker &gdranker = MM::currentMetric()->growthData();
    const int8_t sex = int8_t( _ui->sexComboBox->currentData().toInt());
    const int ethn = _ui->ethnicityComboBox->currentData().toInt();
    const QString src = _ui->sourceComboBox->currentText();
    const GD *gd = gdranker.lookup( sex, ethn, src);
    assert(gd); // Must exist or couldn't have selected
    gdranker.setCurrent( gd);
    _doOnClickedRegion();
}   // end _doOnSelectSource


void MetricsDialog::reflectCurrentMetricStats()
{
    const GD *gd = nullptr;
    const MC::Ptr mc = MM::currentMetric();
    if ( mc)
        gd = mc->growthData().current();

    int sexIdx = -1;
    int ethIdx = -1;
    int8_t sexVal = -1;
    int ethVal = -1;
    if ( gd)
    {
        sexVal = gd->sex();
        ethVal = gd->ethnicity();
        sexIdx = _ui->sexComboBox->findData( sexVal);
        ethIdx = _ui->ethnicityComboBox->findData( ethVal);
    }   // end if

    _ui->sexComboBox->setCurrentIndex( sexIdx);
    _ui->ethnicityComboBox->setCurrentIndex( ethIdx);
    _updateSourcesDropdown( sexVal, ethVal);
}   // end reflectCurrentMetricStats


void MetricsDialog::reflectAtypical()
{
    if ( _ui->matchAtypicalToolButton->isChecked())
    {
        const IntSet mids = _getModelMatchedMetrics( -1);
        _refreshDisplayedRows( mids);
        const IntSet hids = _getModelMatchedPhenotypes( mids);
        _refreshPhenotypes( hids);
    }   // end if
}   // end reflectAtypical


bool MetricsDialog::isShowingAtypical() const { return _ui->matchAtypicalToolButton->isChecked();}


const GD* MetricsDialog::_updateSourcesDropdown( int8_t sex, int eth)
{
    // Update sources for the given sex, ethnicity combo.
    GDS matchingGDs;
    const GD *gd = nullptr;
    _ui->sourceComboBox->clear();

    if ( MM::currentMetric())
    {
        const Metric::GrowthDataRanker &gdranker = MM::currentMetric()->growthData();
        matchingGDs = gdranker.lookup( sex, eth);   // Could be empty
        const QStringList slst = Metric::GrowthDataRanker::sources( matchingGDs);
        _ui->sourceComboBox->addItems( slst);

        /*
        std::cout << "The following sources are available:" << std::endl;
        for ( const QString &ref : slst)
            std::cout << ref.toStdString() << std::endl;
        */

        gd = gdranker.current();
        int srcIdx = -1;
        if ( matchingGDs.empty())
            gd = nullptr;
        else
        {
            if ( matchingGDs.count(gd) == 0)    // Have to set a new set of stats
                gd = *matchingGDs.begin();
            srcIdx = _ui->sourceComboBox->findText( gd->source());
        }   // end else
        _ui->sourceComboBox->setCurrentIndex( srcIdx);
    }   // end if

    const bool isAutoStats = _ui->autoStatsCheckBox->isChecked();
    _ui->sexComboBox->setEnabled( !isAutoStats && _ui->sexComboBox->count() > 1);
    _ui->ethnicityComboBox->setEnabled( !isAutoStats && _ui->ethnicityComboBox->count() > 1);
    _ui->sourceComboBox->setEnabled( !isAutoStats && _ui->sourceComboBox->count() > 1);
    _ui->inPlaneCheckBox->setEnabled( !isAutoStats);

    return gd;
}   // end _updateSourcesDropdown


void MetricsDialog::_doOnClickedRegion()
{
    const int hid = _ui->hpoComboBox->currentData().toInt();
    const IntSet mids = _getModelMatchedMetrics( hid);
    const int nmid = _refreshDisplayedRows( mids);
    _setCurrentMetric( nmid);
    const IntSet hids = _getModelMatchedPhenotypes( mids);
    _refreshPhenotypes( hids);
}   // end _doOnClickedRegion


void MetricsDialog::_doOnClickedType() { _doOnClickedRegion();}


void MetricsDialog::_doOnClickedAtypical()
{
    _ui->hpoComboBox->setCurrentIndex( 0);  // Set to --any--
    _doOnClickedRegion();
}   // end _doOnClickedAtypical


void MetricsDialog::_doOnClickedForceInPlane()
{
    MM::setInPlane( _ui->inPlaneCheckBox->isChecked());
    _doOnClickedRegion();
}   // end _doOnClickedForceInPlane


void MetricsDialog::selectHPO( int hid)
{
    _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData( hid));
    _setCurrentMetric( _refreshDisplayedRows( _getModelMatchedMetrics( hid)));
}   // end selectHPO


void MetricsDialog::_doOnClickedPhenotype()
{
    const int hid = _ui->hpoComboBox->currentData().toInt();
    _setCurrentMetric( _refreshDisplayedRows( _getModelMatchedMetrics( hid)));
    if ( hid != -1)
        emit onSelectedHPOTerm( hid);
}   // end _doOnClickedPhenotype


// Not called for now
void MetricsDialog::_doOnClickedSyndrome()
{
    const IntSet mids = _getModelMatchedMetrics( -1);
    _refreshDisplayedRows( mids);
    const IntSet hids = _getModelMatchedPhenotypes( mids);
    _refreshPhenotypes( hids);
}   // end _doOnClickedSyndrome


// Not called for now
void MetricsDialog::_doOnClickedFlipCombosButton()
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
    QToolTip::showText( _ui->flipButton->mapToGlobal(QPoint()), toolTipText);
    //_refreshDisplayedRows();
}   // end _doOnClickedFlipCombosButton
