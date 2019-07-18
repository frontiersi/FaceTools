/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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
#include <MetricsDialog.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowMetrics : public FaceAction
{ Q_OBJECT
public:
    ActionShowMetrics( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Show the measurements dialog and indicate on the face where the measurements are taken from.";}

    QWidget* widget() const override { return _mdialog;}

    void setShowScanInfoAction( QAction*);

    static void setOpacityOnShow( double);

protected:
    void postInit() override;
    void purge( const FM*, Event) override;
    bool checkState( Event) override;
    bool checkEnable( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;

private slots:
    void _doOnSelectedMetric(int);
    void _doOnSetMetricGrowthData();
    void _doOnChangedMetric(int);

private:
    static double s_opacity;
    Widget::MetricsDialog *_mdialog;
    bool _nowShowing;
    std::unordered_map<FMV*, vtkNew<vtkTextActor> > _texts;
    void _updateMetricText(int);
    void _addViewer( FMV*);
};  // end class

}}   // end namespaces

#endif
