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

#include <FaceTypes.h>
#include <QtCharts/QChartView>
#include <QDialog>

namespace Ui { class ChartDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ChartDialog : public QDialog
{ Q_OBJECT
public:
    explicit ChartDialog( QWidget *parent = nullptr);
    ~ChartDialog() override;

    // Refresh against the currently selected model and active metric.
    void refresh();

public slots:
    void doOnSetEthnicityIgnored( bool);

private slots:
    void doOnUserSelectedEthnicity();
    void doOnUserSelectedSource();
    void doOnEditData();
    void doOnSetMetricActive();
    void doOnResetChart();

private:
    Ui::ChartDialog *_ui;
    QtCharts::QChartView *_cview;
    bool _ignEthn;

    void populateSources();
    void populateEthnicites();
    void populateSexs();
    void populateDimensions();
    const FM* isDemographic() const;
};  // end class

}}   // end namespaces

#endif
