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

#ifndef FACETOOLS_WIDGET_METRICS_DISPLAY_DIALOG_H
#define FACETOOLS_WIDGET_METRICS_DISPLAY_DIALOG_H

#include <FaceTypes.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class MetricsDisplayDialog; }

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT MetricsDisplayDialog : public QDialog
{ Q_OBJECT
public:
    explicit MetricsDisplayDialog( QWidget *parent = nullptr);
    ~MetricsDisplayDialog() override;

    void setShowScanInfoAction( QAction*);
    void setShowMetricsAction( QAction*);
    void setShowChartAction( QAction*);

    // Populate from statically accessed SyndromeManager, HPOTermManager, and MetricCalculatorManager.
    void populate();

signals:
    void onChangeOpacity( double);
    void onSelectMetric( int);  // Metric ID

public slots:
    void setMetricSelected( int);   // Pass in metric ID
    void setRowSelected( int);      // Pass in row ID

private slots:
    void doOnResizeColumns();
    void doOnRowChanged( QTableWidgetItem*);
    void doOnUserSelectedSyndrome();
    void doOnUserSelectedHPOTerm();

private:
    Ui::MetricsDisplayDialog *ui;
    std::unordered_map<int, int> _mIdRows;  // Metric ID --> Row index

    void appendRow( int);
    void updateRow( int);
    void populateHPOs( const IntSet&);
    void populateSyndromes( const IntSet&);
};  // end class

}}  // end namespaces

#endif
