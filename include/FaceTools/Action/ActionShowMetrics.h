/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <FaceTools/Widget/ChartDialog.h>
#include <FaceTools/Widget/MetricsDialog.h>
#include <vtkTextActor.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionShowMetrics : public FaceAction
{ Q_OBJECT
public:
    ActionShowMetrics( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Display the facial measurements.";}

    QWidget* widget() const override { return _mdialog;}

    static void setParallelProjectionOnShow( bool);

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private slots:
    void _doOnMatchHPOs();
    void _doOnRemeasure();
    void _doOnSelectMetric( int, int);
    void _doRefreshAllMetricsVisibility();
    void _doRefreshMetricVisibility( int);

private:
    static bool s_showParallelProjection;
    Widget::MetricsDialog *_mdialog;
    Widget::ChartDialog *_cdialog;
    std::unordered_map<const FMV*, vtkNew<vtkTextActor> > _texts;
    bool _updateText( const Vis::FV*, int);
    void _setParallelProjection( bool);
    void _refreshMetricAppearance( FM*, int);
    void _refreshMetricVisibility( int);
    void _updateCurrentMetricDisplayedInfo();
};  // end class

}}   // end namespaces

#endif
