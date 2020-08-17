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

#ifndef FACETOOLS_WIDGET_METRICS_DIALOG_H
#define FACETOOLS_WIDGET_METRICS_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QTableWidgetItem>
#include <QAction>
#include <QDialog>

namespace Ui { class MetricsDialog; }

namespace FaceTools {

namespace Metric { class GrowthData;}

namespace Widget {

class FaceTools_EXPORT MetricsDialog : public QDialog
{ Q_OBJECT
public:
    explicit MetricsDialog( QWidget *parent = nullptr);
    ~MetricsDialog() override;

    void setShowScanInfoAction( QAction*);
    void setShowPhenotypesAction( QAction*);

    // Returns true iff the atypical warning triangle is checked.
    bool isShowingAtypical() const;

signals:
    void onShowChart();
    void onStatsChanged();
    void onRefreshAllMetrics();
    void onSelectedHPOTerm( int);
    void onUpdateMatchingPhenotypes( const IntSet&);

public slots:
    void show();
    void reflectCurrentMetricStats();
    void reflectAtypical();
    void selectHPO( int);

protected:
    void closeEvent( QCloseEvent*) override;

private slots:
    void _doOnSetAllChecked(bool);
    void _doSortOnColumn( int);
    void _doOnItemChanged( QTableWidgetItem*);
    void _doOnChangeTableRow( QTableWidgetItem*);
    void _doOnClickedFlipCombosButton();
    void _doOnClickedPhenotype();
    void _doOnClickedAutoStats();
    void _doOnSelectEthnicity();
    void _doOnSelectSex();
    void _doOnSelectSource();
    void _doOnClickedRegion();
    void _doOnClickedType();
    void _doOnClickedSyndrome();
    void _doOnClickedAtypical();
    void _doOnClickedForceInPlane();

private:
    Ui::MetricsDialog *_ui;
    std::unordered_map<int, int> _idRows;  // Metric ID --> Row index
    bool _syndromeToPhenotype;  // Track combo box reordering
    int _prowid;

    void _appendRow( int);
    void _highlightRow( int);
    void _populateSyndromes();
    void _refreshPhenotypes( const IntSet&);
    void _populateTable();
    void _populateMetricType();
    void _populateRegionType();
    void _refreshAtypical();
    IntSet _getModelMatchedMetrics( int) const;
    IntSet _getModelMatchedPhenotypes( const IntSet&) const;
    int _refreshDisplayedRows( const IntSet&);
    void _setCurrentMetric( int);
    void _onSelectSexAndEthnicity( int8_t, int);
    void _refreshAvailableSexesFromMetric();
    void _refreshAvailableEthnicitiesFromMetric();
    const Metric::GrowthData *_updateSourcesDropdown( int8_t, int);
};  // end class

}}  // end namespaces

#endif
