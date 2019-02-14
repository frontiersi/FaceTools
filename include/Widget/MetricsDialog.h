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

#ifndef FACETOOLS_WIDGET_METRICS_DIALOG_H
#define FACETOOLS_WIDGET_METRICS_DIALOG_H

#include <FaceTypes.h>
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
    void setShowMetricsAction( QAction*);
    void setDetectedOnlyEnabled( bool);

signals:
    void onShowPhenotypes();
    void onShowChart();
    // Emitted whenever the ethnicity ignored checkbox is changed, and immediately before onMatchedPhenotypes is emitted.
    void onEthnicityIgnored( bool);
    // Emit a refreshed set of phenotype IDs that match to the currently selected model.
    void onShowingPhenotypes( const IntSet&);
    void onRefreshedMetrics();

public slots:
    void refresh();

private slots:
    void doOnUserSelectedSyndrome();
    void doOnUserSelectedHPOTerm();
    void doOnSetAllChecked(bool);
    void sortOnColumn( int);
    void doOnItemChanged( QTableWidgetItem*);
    void doOnSetCurrentMetric( int);   // Pass in metric ID
    void doOnClickedFlipCombosButton();
    void doOnEthnicityIgnored();
    void doOnClickedMatchButton();

private:
    Ui::MetricsDialog *_ui;
    std::unordered_map<int, int> _idRows;  // Metric ID --> Row index
    bool _syndromeToPhenotype;  // Track combo box reordering
    IntSet _mpids;  // Matched set of phenotype IDs

    void appendRow( int);
    void populateHPOs( const IntSet&);
    void populateSyndromes( const IntSet&);
    void resetIdRowMap();
    void highlightRow( int);
    void setSelectedRow( int); // Pass in row ID
    void populate();
};  // end class

}}  // end namespaces

#endif
