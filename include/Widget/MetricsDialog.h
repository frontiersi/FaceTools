/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <FaceTypes.h>
#include <ChartDialog.h>
#include <PhenotypesDialog.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class MetricsDialog; }

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT MetricsDialog : public QDialog
{ Q_OBJECT
public:
    explicit MetricsDialog( QWidget *parent = nullptr);
    ~MetricsDialog() override;

    void setShowScanInfoAction( QAction*);
    void highlightRow( int metricId);

signals:
    void onSelectedMetric( int);
    void onSetMetricGrowthData();
    void onChangedMetricVisibility( int, bool);

public slots:
    void show();
    void hide();
    void refresh();

protected:
    void closeEvent( QCloseEvent*) override;

private slots:
    void _doOnUserSelectedSyndrome();
    void _doOnUserSelectedHPOTerm();
    void _doOnSetAllChecked(bool);
    void _sortOnColumn( int);
    void _doOnItemChanged( QTableWidgetItem*);
    void _doOnClickedFlipCombosButton();
    void _doOnClickedMatchButton();
    void _doOnClickedUseSubjectDemographicsButton();
    void _doOnShowPhenotypes();
    void _doOnShowChart();

private:
    Ui::MetricsDialog *_ui;
    std::unordered_map<int, int> _idRows;  // Metric ID --> Row index
    bool _syndromeToPhenotype;  // Track combo box reordering
    IntSet _mpids;  // Matched set of phenotype IDs

    PhenotypesDialog *_pdialog;
    ChartDialog *_cdialog;

    void _appendRow( int);
    void _populateHPOs( const IntSet&);
    void _populateSyndromes( const IntSet&);
    void _resetIdRowMap();
    void _setSelectedRow( int); // Pass in row ID
    void _populate();
    void _refreshPhenotypeList();
};  // end class

}}  // end namespaces

#endif
