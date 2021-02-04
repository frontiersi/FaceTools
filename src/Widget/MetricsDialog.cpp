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

#include <Widget/MetricsDialog.h>
#include <ui_MetricsDialog.h>
#include <Widget/CheckAllTableHeader.h>
#include <Widget/IntTableWidgetItem.h>
#include <Metric/MetricManager.h>
#include <Metric/SyndromeManager.h>
#include <Metric/PhenotypeManager.h>
#include <Metric/StatsManager.h>
#include <MiscFunctions.h>
#include <ModelSelect.h>
#include <Ethnicities.h>
#include <QHeaderView>
#include <QToolTip>
#include <QComboBox>
using FaceTools::Widget::IntTableWidgetItem;
using FaceTools::Widget::MetricsDialog;
using HPOMan = FaceTools::Metric::PhenotypeManager;
using SynMan = FaceTools::Metric::SyndromeManager;
using MM = FaceTools::Metric::MetricManager;
using MC = FaceTools::Metric::Metric;
using MS = FaceTools::ModelSelect;
using SM = FaceTools::Metric::StatsManager;


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
    QDialog(parent), _ui(new Ui::MetricsDialog), _cmid(-1)
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
    _resetHPOsComboBox( HPOMan::ids());
    _populateRegionType();
    _populateMetricType();
    _populateTable();

    connect( _ui->typeComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedTypeOrRegion);
    connect( _ui->regionComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedTypeOrRegion);

    //connect( _ui->synComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedSyndrome);
    connect( _ui->atypicalToolButton, &QToolButton::toggled, this, &MetricsDialog::_doOnClickedAtypical);

    connect( _ui->hpoComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnClickedHPO);
    connect( _ui->matchSubjectCheckBox, &QCheckBox::clicked, this, &MetricsDialog::_doOnClickedMatchSubject);

    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectSexOrEthnicity);
    connect( _ui->ethnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectSexOrEthnicity);

    connect( _ui->sourceComboBox, QOverload<int>::of(&QComboBox::activated), this, &MetricsDialog::_doOnSelectSource);
    connect( _ui->inPlaneCheckBox, &QCheckBox::clicked, this, &MetricsDialog::_doOnClickedInPlane);

    _ui->table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    _ui->table->setColumnWidth( SHOW_COL, 18);

    connect( _ui->table, &QTableWidget::currentItemChanged, this, &MetricsDialog::_doOnChangeTableRow);
    connect( _ui->table, &QTableWidget::itemChanged, this, &MetricsDialog::_doOnItemChanged);
    connect( _ui->showChartButton, &QToolButton::clicked, this, &MetricsDialog::onShowChart);
    _doOnChangeTableRow( _ui->table->item(0,0));
}   // end ctor


MetricsDialog::~MetricsDialog() { delete _ui;}


void MetricsDialog::_populateTable()
{
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


void MetricsDialog::setShowScanInfoAction( QAction *act)
{
    _ui->showScanInfoButton->setDefaultAction( act);
    //connect( act, &QAction::toggled, _ui->showScanInfoButton, &QAbstractButton::setChecked);
    _ui->showScanInfoButton->setCheckable(true);
    connect( act, &QAction::toggled, [this]( bool v){ _ui->showScanInfoButton->setChecked(v);});
}   // end setShowScanInfoAction


void MetricsDialog::setShowHPOsAction( QAction *act)
{
    _ui->showHPOsButton->setDefaultAction( act);
    //connect( act, &QAction::toggled, _ui->showHPOsButton, &QAbstractButton::setChecked);
    _ui->showHPOsButton->setCheckable(true);
    connect( act, &QAction::toggled, [this]( bool v){ _ui->showHPOsButton->setChecked(v);});
}   // end setShowHPOsAction


void MetricsDialog::closeEvent( QCloseEvent *e)
{
    e->accept();
    accept();
}   // end closeEvent


void MetricsDialog::hideEvent( QHideEvent* e)
{
    _ui->atypicalToolButton->setChecked(false);
    QDialog::hideEvent(e);
}   // end hideEvent


void MetricsDialog::showEvent( QShowEvent *e)
{
    positionWidgetToSideOfParent(this);
    QDialog::showEvent(e);
    _doOnClickedHPO();
}   // end showEvent


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
    emit onRefreshAllMetricsVisibility();
}   // end _doOnSetAllChecked


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
    emit onRefreshMetricVisibility( mid);
}   // end _doOnItemChanged


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
    _setMetric( _ui->table->item( item->row(), IDNT_COL)->text().toInt());
}   // end _doOnChangeTableRow


// Reset the HPOs combo box to be just the provided entries.
// Tries to keep the previously selected term as the current index.
int MetricsDialog::_resetHPOsComboBox( const IntSet &hids)
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
    QSignalBlocker b0( _ui->hpoComboBox);
    _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData( hid));
    return hid;
}   // end _resetHPOsComboBox


void MetricsDialog::_populateSyndromes()
{
    _ui->synComboBox->clear();
    _ui->synComboBox->addItem( ANY_ITEM, -1);

    const IntSet *sids = &SynMan::ids();
    const int hid = _ui->hpoComboBox->currentData().toInt();
    if ( hid >= 0)
        sids = &SynMan::hpoSyndromes(hid);

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
    QSignalBlocker b0( _ui->synComboBox);
    _ui->synComboBox->setCurrentIndex( 0);
}   // end _populateSyndromes


void MetricsDialog::_appendRow( int mid)
{
    const MC *mc = MM::cmetric( mid);
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
        const MC *mc = MM::cmetric(mid);
        if ( (region.isEmpty() || mc->region() == region) && (mtype.isEmpty() || mc->category() == mtype))
            mids.insert(mid);
    }   // end for
    return mids;
}   // end filterMetrics


// Return just those HPOs from hids that are associated with metrics in mids
IntSet filterHPOTermsByMetrics( const IntSet &hids, const IntSet &mids)
{
    IntSet hids1;
    for ( int hid : hids)
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
    IntSet hids1;
    FaceTools::FM::RPtr fm = MS::selectedModelScopedRead();
    for ( int hid : hids)
        if ( !fm || HPOMan::phenotype(hid)->isPresent(*fm, -1))
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


IntSet MetricsDialog::_getModelMatchedMetrics( int hid) const
{
    const QString mtype = _ui->typeComboBox->currentIndex() > 0 ? _ui->typeComboBox->currentText() : "";
    const QString region = _ui->regionComboBox->currentIndex() > 0 ? _ui->regionComboBox->currentText() : "";
    IntSet mids = filterMetrics( hid >= 0 ? HPOMan::phenotype(hid)->metrics() : MM::ids(), mtype, region);
    if ( _ui->atypicalToolButton->isChecked())
    {
        const IntSet hids = filterHPOTermsByModelMatched( HPOMan::ids());
        mids = filterMetricsByHPOTerms( mids, hids);
    }   // end if
    return mids;
}   // end _getModelMatchedMetrics


int MetricsDialog::_refreshDisplayedRows( const IntSet &mids)
{
    int smid = -1;

    for ( int i = 0; i < _ui->table->rowCount(); ++i)
    {
        const int mid = _ui->table->item(i, IDNT_COL)->text().toInt();
        MC::Ptr mc = MM::metric(mid);

        if ( mids.count(mid) > 0)  // Should show this one
        {
            if ( smid < 0 || mid == _cmid)
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


void MetricsDialog::_highlightRow( int omid, int mid)
{
    const int prowid = omid >= 0 ? _idRows.at(omid) : -1;
    const int crowid =  mid >= 0 ? _idRows.at( mid) : -1;
    _ui->table->blockSignals(true);

    if ( prowid >= 0)
    {
        static const QBrush wbrush( Qt::white);
        _ui->table->item( prowid, SHOW_COL)->setBackground( wbrush);
        _ui->table->item( prowid, IDNT_COL)->setBackground( wbrush);
        _ui->table->item( prowid, NAME_COL)->setBackground( wbrush);
        _ui->table->item( prowid, DESC_COL)->setBackground( wbrush);
    }   // end if

    if ( crowid >= 0)
    {
        static const QBrush BG( QColor(200,235,255));
        _ui->table->item( crowid, SHOW_COL)->setBackground( BG);
        _ui->table->item( crowid, IDNT_COL)->setBackground( BG);
        _ui->table->item( crowid, NAME_COL)->setBackground( BG);
        _ui->table->item( crowid, DESC_COL)->setBackground( BG);
        _ui->table->setCurrentCell( crowid, SHOW_COL);
    }   // end if

    _ui->table->blockSignals(false);
}   // end _highlightRow


bool MetricsDialog::_setMetric( int mid)
{
    if ( mid == _cmid)
        return false;

    const MC *mc = MM::cmetric( mid);

    // Set the generic type remarks if no measurement specific remarks available.
    QString rmks;
    if ( mc)
        rmks = mc->remarks().isEmpty() ? mc->typeRemarks() : mc->remarks();
    _ui->remarksTextBrowser->setHtml( rmks);

    _highlightRow( _cmid, mid);
    const int omid = _cmid;
    _cmid = mid;

    // Reset the sexes available in statistics of the new metric
    _ui->sexComboBox->clear();
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

    // Reset the ethnicities available in statistics of the new metric
    _ui->ethnicityComboBox->clear();
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

    refreshMetric();
    emit onSelectMetric( omid, mid);
    return true;
}   // end _setMetric


void MetricsDialog::refreshMetric()
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    SM::RPtr gd = SM::stats( _cmid, fm.get());

    int ethVal = 0;
    int8_t sexVal = UNKNOWN_SEX;
    QString src;
    if ( gd)
    {
        sexVal = gd->sex();
        ethVal = gd->ethnicity();
        src = gd->source();
    }   // end if

    const int sexIdx = _ui->sexComboBox->findData( sexVal);
    const int ethIdx = _ui->ethnicityComboBox->findData( ethVal);

    QSignalBlocker b0( _ui->sexComboBox);
    QSignalBlocker b1( _ui->ethnicityComboBox);
    QSignalBlocker b2( _ui->sourceComboBox);
    QSignalBlocker b3( _ui->inPlaneCheckBox);
    QSignalBlocker b4( _ui->matchSubjectCheckBox);

    _ui->sexComboBox->setCurrentIndex( sexIdx);
    _ui->ethnicityComboBox->setCurrentIndex( ethIdx);
    _refreshSources( sexVal, ethVal);
    const int srcIdx = _ui->sourceComboBox->findText( src);
    _ui->sourceComboBox->setCurrentIndex( srcIdx);
    const MC *mc = MM::cmetric(_cmid);
    _ui->inPlaneCheckBox->setChecked( mc && mc->inPlane( fm.get()));
    const bool usingSubjectMatchedStats = !SM::usingDefaultMetricStats( _cmid);
    _ui->matchSubjectCheckBox->setChecked( usingSubjectMatchedStats);

    _ui->sexComboBox->setEnabled( !usingSubjectMatchedStats && _ui->sexComboBox->count() > 1);
    _ui->ethnicityComboBox->setEnabled( !usingSubjectMatchedStats && _ui->ethnicityComboBox->count() > 1);
    _ui->sourceComboBox->setEnabled( !usingSubjectMatchedStats && _ui->sourceComboBox->count() > 1);
    _ui->inPlaneCheckBox->setEnabled( !usingSubjectMatchedStats && mc && !mc->fixedInPlane());
    _ui->matchSubjectCheckBox->setEnabled( gd != nullptr);
}   // end refreshMetric


void MetricsDialog::_doOnSelectSexOrEthnicity()
{
    assert( !_ui->matchSubjectCheckBox->isChecked());
    int8_t sex = int8_t( _ui->sexComboBox->currentData().toInt());
    const int ethn = _ui->ethnicityComboBox->currentData().toInt();

    const MC *mc = MM::cmetric(_cmid);
    if ( !mc || mc->growthData().compatible( sex, ethn).empty())
    {
        QSignalBlocker blocker(_ui->sexComboBox);
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( UNKNOWN_SEX));
        sex = UNKNOWN_SEX;
    }   // end if

    _refreshSources( sex, ethn);
    _doOnSelectSource();
}   // end _doOnSelectSexOrEthnicity


void MetricsDialog::_refreshSources( int8_t sex, int eth)
{
    _ui->sourceComboBox->clear();
    Metric::GrowthDataSources mgds;
    const MC *mc = MM::cmetric(_cmid);
    if ( mc)
        mgds = mc->growthData().compatible( sex, eth);   // Could be empty
    const QStringList slst = Metric::GrowthDataRanker::sources( mgds);
    _ui->sourceComboBox->addItems( slst);
}   // end _refreshSources


bool MetricsDialog::atypicalOnly() const { return _ui->atypicalToolButton->isChecked();}


void MetricsDialog::refreshMatched()
{
    const IntSet &mids = _getModelMatchedMetrics( _ui->hpoComboBox->currentData().toInt());
    _setMetric( _refreshDisplayedRows( mids));

    const int sid = _ui->synComboBox->currentData().toInt(); // Get HPO terms that match selected syndrome
    const IntSet &ihids = sid < 0 ? HPOMan::ids() : SynMan::syndrome(sid)->hpos();
    // Filter HPO terms further by matched metrics (only those with an associated metric also in mids).
    IntSet hids = filterHPOTermsByMetrics( ihids, mids);
    // Further filter HPO terms by only those that match the current model (if option checked)
    if ( _ui->atypicalToolButton->isChecked())
        hids = filterHPOTermsByModelMatched( hids);

    const int hid = _resetHPOsComboBox( hids);
    if ( hid != -1)
        emit onSelectHPO( hid);
    emit onMatchHPOs( hids);
}   // end refreshMatched


void MetricsDialog::_doOnSelectSource()
{
    assert( !_ui->matchSubjectCheckBox->isChecked());
    const int8_t sex = int8_t( _ui->sexComboBox->currentData().toInt());
    const int ethn = _ui->ethnicityComboBox->currentData().toInt();
    const QString src = _ui->sourceComboBox->currentText();
    SM::setDefaultMetricStats( _cmid, sex, ethn, src);
    refreshMatched();
}   // end _doOnSelectSource


void MetricsDialog::_doOnClickedTypeOrRegion()
{
    refreshMatched();
}   // end _doOnClickedTypeOrRegion


void MetricsDialog::_doOnClickedAtypical()
{
    QSignalBlocker b0( _ui->hpoComboBox);
    QSignalBlocker b1( _ui->typeComboBox);
    QSignalBlocker b2( _ui->regionComboBox);
    _ui->hpoComboBox->setCurrentIndex( 0);  // Set to --any--
    _ui->typeComboBox->setCurrentIndex( 0);
    _ui->regionComboBox->setCurrentIndex( 0);
    refreshMatched();
}   // end _doOnClickedAtypical


void MetricsDialog::_doOnClickedMatchSubject() // Using model matched stats active by default
{
    SM::setUseDefaultMetricStats( _cmid, !_ui->matchSubjectCheckBox->isChecked());
    refreshMetric();
    emit onRemeasure();
    refreshMatched();
}   // end _doOnClickedMatchSubject


void MetricsDialog::_doOnClickedInPlane()
{
    MC::Ptr mc = MM::metric(_cmid);
    assert(mc);
    mc->setInPlane( _ui->inPlaneCheckBox->isChecked());
    emit onRemeasure();
    refreshMatched();
}   // end _doOnClickedInPlane


void MetricsDialog::selectHPO( int hid)
{
    QSignalBlocker b0( _ui->hpoComboBox);
    _ui->hpoComboBox->setCurrentIndex( _ui->hpoComboBox->findData( hid));
    _setMetric( _refreshDisplayedRows( _getModelMatchedMetrics( hid)));
}   // end selectHPO


void MetricsDialog::_doOnClickedHPO()
{
    const int hid = _ui->hpoComboBox->currentData().toInt();
    _setMetric( _refreshDisplayedRows( _getModelMatchedMetrics( hid)));
    if ( hid != -1)
        emit onSelectHPO( hid);
}   // end _doOnClickedHPO

