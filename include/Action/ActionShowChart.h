/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_ACTION_ACTION_SHOW_CHART_H
#define FACE_TOOLS_ACTION_ACTION_SHOW_CHART_H

#include "FaceAction.h"
#include <MetricCalculator.h>
#include <QtCharts/QChartView>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionShowChart : public FaceAction
{ Q_OBJECT
public:
    ActionShowChart( const QString&, const QIcon&, QWidget *parent=nullptr);

    QWidget* getWidget() const override { return _dialog;}

public slots:
    void setMetric( int metricId);

protected slots:
    bool testReady( const Vis::FV*) override { return true;}
    void tellReady( const Vis::FV*, bool) override;
    bool testEnabled( const QPoint*) const override;
    bool testIfCheck( const Vis::FV*) const override { return _dialog->isVisible();}
    bool doAction( FVS&, const QPoint&) override;
    void purge( const FM*) override;

private:
    QDialog* _dialog;
    QtCharts::QChartView *_cview;
    int _mid;   // ID of the metric currently being displayed
    void resetChart( const FM* fm=nullptr);
};  // end class

}}   // end namespaces

#endif
