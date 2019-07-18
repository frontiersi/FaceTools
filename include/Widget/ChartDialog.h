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

#ifndef FACETOOLS_WIDGET_CHART_DIALOG_H
#define FACETOOLS_WIDGET_CHART_DIALOG_H

#include <FaceTools.h>
#include <Chart.h>
#include <QDialog>

namespace Ui { class ChartDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ChartDialog : public QDialog
{ Q_OBJECT
public:
    explicit ChartDialog( QWidget *parent = nullptr);
    ~ChartDialog() override;

signals:
    void onGrowthDataChanged();

public slots:
    // Call refreshMetricOrModel() whenever a model or a metric is selected or when
    // the demographics of the selected model are changed. This will check if the
    // metric growth data should change and update the chart itself. If the metric
    // growth data changes for the currently selected metric, signal
    // onSetMetricGrowthData is emitted with the metric id.
    void refreshMetricOrModel();

private slots:
    void _doOnUserSelectedSex();
    void _doOnUserSelectedEthnicity();
    void _doOnSaveImage();
    void _updateChart();

private:
    Ui::ChartDialog *_ui;
    QtCharts::QChartView *_cview;

    int8_t _selectedSex() const;
    int _selectedEthnicity() const;
    QString _selectedSource() const;
    void _populateSexs( int);
    void _populateEthnicities( int8_t);
    void _populateSources();
    void _populateDimensions();
    void _setEthnicity();
    void _setSex();
};  // end class

}}   // end namespaces

#endif
