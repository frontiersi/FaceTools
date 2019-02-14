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

#ifndef FACE_TOOLS_ACTION_ACTION_SHOW_METRICS_H
#define FACE_TOOLS_ACTION_ACTION_SHOW_METRICS_H

#include "FaceAction.h"
#include <MetricsInteractor.h>
#include <MetricsDialog.h>
#include <ChartDialog.h>
#include <PhenotypesDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowMetrics : public FaceAction
{ Q_OBJECT
public:
    ActionShowMetrics( const QString&, const QIcon&, QWidget *parent=nullptr);

    QWidget* getWidget() const override { return _mdialog;}

    void setShowScanInfoAction( QAction*);

protected slots:
    bool testReady( const Vis::FV*) override;
    void tellReady( const Vis::FV*, bool) override;
    bool testEnabled( const QPoint*) const override;
    bool testIfCheck( const Vis::FV*) const override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(VIEW_CHANGE);}
    void purge( const FM*) override;

private slots:
    void doOnMetricUpdated(int);
    void doOnSetSelectedMetric();
    void doOnShowChartDialog();
    void doOnShowPhenotypesDialog();
    bool doOnRefresh();

private:
    Widget::MetricsDialog *_mdialog;
    Widget::ChartDialog *_cdialog;
    Widget::PhenotypesDialog *_pdialog;
    FMS _vmodels;
};  // end class

}}   // end namespaces

#endif
