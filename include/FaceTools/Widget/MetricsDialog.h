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

#ifndef FACETOOLS_WIDGET_METRICS_DIALOG_H
#define FACETOOLS_WIDGET_METRICS_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QTableWidgetItem>
#include <QAction>
#include <QDialog>

namespace Ui { class MetricsDialog; }

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT MetricsDialog : public QDialog
{ Q_OBJECT
public:
    explicit MetricsDialog( QWidget *parent = nullptr);
    ~MetricsDialog() override;

    void setShowScanInfoAction( QAction*);
    void setShowHPOsAction( QAction*);

    // Return the currently selected metric (-1 if none).
    inline int currentMetricId() const { return _cmid;}

    void refreshMetric();
    void refreshMatched();

    // Returns true iff the atypical warning triangle is checked.
    bool atypicalOnly() const;

signals:
    void onRefreshAllMetricsVisibility();  // Was onRefreshAllMetrics
    void onRefreshMetricVisibility( int);
    void onSelectMetric( int, int);    // Old metric ID --> new metric ID
    void onRemeasure(); // Emitted if must remeasure *all* models for the current metric
    void onShowChart();
    void onSelectHPO( int);
    void onMatchHPOs( const IntSet&);

public slots:
    void selectHPO( int);

protected:
    void showEvent( QShowEvent*) override;
    void closeEvent( QCloseEvent*) override;
    void hideEvent( QHideEvent*) override;

private slots:
    void _doOnSetAllChecked(bool);
    void _doSortOnColumn( int);
    void _doOnItemChanged( QTableWidgetItem*);
    void _doOnChangeTableRow( QTableWidgetItem*);
    void _doOnClickedHPO();
    void _doOnSelectSexOrEthnicity();
    void _doOnSelectSource();
    void _doOnClickedTypeOrRegion();
    void _doOnClickedAtypical();
    void _doOnClickedMatchSubject();
    void _doOnClickedInPlane();

private:
    Ui::MetricsDialog *_ui;
    std::unordered_map<int, int> _idRows;  // Metric ID --> Row index
    int _cmid;  // Current metric ID

    void _populateSyndromes();
    void _populateMetricType();
    void _populateRegionType();
    void _populateTable();

    int _resetHPOsComboBox( const IntSet&);
    void _appendRow( int);
    void _highlightRow( int, int);

    IntSet _getModelMatchedMetrics( int) const;
    int _refreshDisplayedRows( const IntSet&);
    bool _setMetric( int);
    void _refreshSources( int8_t, int);
};  // end class

}}  // end namespaces

#endif
