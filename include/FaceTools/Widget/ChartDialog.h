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

#ifndef FACETOOLS_WIDGET_CHART_DIALOG_H
#define FACETOOLS_WIDGET_CHART_DIALOG_H

#include <FaceTools.h>
#include <QtCharts/QChartView>
#include <QFileDialog>
#include <QDialog>

namespace Ui { class ChartDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ChartDialog : public QDialog
{ Q_OBJECT
public:
    explicit ChartDialog( QWidget *parent = nullptr);
    ~ChartDialog() override;

public slots:
    void refresh();

private slots:
    void _doOnSaveImage();
    void _doRefreshChart();

private:
    Ui::ChartDialog *_ui;
    QtCharts::QChartView *_cview;
    QFileDialog *_fdialog;

    void _refreshDimensions();
};  // end class

}}   // end namespaces

#endif
