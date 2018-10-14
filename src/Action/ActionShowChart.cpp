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

#include <ActionShowChart.h>
#include <MetricCalculatorManager.h>
#include <MetricVisualiser.h>
#include <FaceModel.h>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <QVBoxLayout>
#include <algorithm>
#include <cassert>
using FaceTools::Action::ActionShowChart;
using FaceTools::Action::FaceAction;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::MC;
using FaceTools::Vis::FV;
using FaceTools::FVS;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;


ActionShowChart::ActionShowChart( const QString& dn, const QIcon& ico, QWidget *parent)
    : FaceAction( dn, ico), _dialog( new QDialog( parent)), _cview( new QtCharts::QChartView), _mid(-1)
{
    setCheckable( true, false);
    _cview->setRenderHint( QPainter::Antialiasing);
    _dialog->setWindowTitle( parent->windowTitle() + " | Metric Growth Chart");
    _dialog->resize(700,500);
    _dialog->setLayout(new QVBoxLayout);
    _dialog->setContentsMargins(0,0,0,0);
    _dialog->layout()->addWidget(_cview);

    if ( MCM::count() > 0)
        _mid = *MCM::ids().begin();
    resetChart();
}   // end ctor


// public slot
void ActionShowChart::setMetric( int mid)
{
    if ( MCM::metric( mid) != nullptr)
    {
        _mid = mid;
        const FV* fv = ready1();
        resetChart( fv ? fv->data() : nullptr);
    }   // end if
}   // end setMetric


// If metrics are loaded in MCM, it should always be the case that the chart is set.
bool ActionShowChart::testEnabled( const QPoint*) const { return _cview->chart() != nullptr;}


bool ActionShowChart::doAction( FVS&, const QPoint&)
{
    _dialog->show();
    _dialog->raise();
    _dialog->activateWindow();
    return true;
}   // end doAction


void ActionShowChart::tellReady( const FV* fv, bool isready)
{
    if ( _cview->chart() && _dialog->isVisible())
        resetChart( fv && isready ? fv->data() : nullptr);
}   // end tellReady


namespace {

QtCharts::QScatterSeries* createMetricPoint( MC::Ptr mc, const MetricValue *mv, const QString& title, const QColor& c, double age)
{
    using namespace QtCharts;
    QScatterSeries *dpoints = new QScatterSeries;
    dpoints->setName( title);
    dpoints->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    dpoints->setMarkerSize(10);
    dpoints->setColor(c);

    const size_t dims = mc->dims();
    for ( size_t i = 0; i < dims; ++i)
    {
        const rlib::RSD::Ptr rsd = mc->type()->rsd(i);
        dpoints->append( age, mv->values()[i]);
    }   // end for
    return dpoints;
}   // end createMetricPoint

}   // end namespace


void ActionShowChart::resetChart( const FM* fm)
{
    MC::Ptr mc = MCM::metric(_mid);
    if ( mc == nullptr)
        return;

    using namespace QtCharts;
    QChart *chart = new QChart;
    _cview->setChart(chart);

    // Add the metric data to the chart
    double xmin, xmax;
    mc->addSeriesToChart( chart, &xmin, &xmax);

    if ( fm)
    {
        const double age = fm->age();
        xmin = std::min<double>(xmin, age);
        xmax = std::max<double>(xmax, age);

        if ( mc->isBilateral())
        {
            const MetricValue *mvl = fm->metricsL().get( _mid);
            const MetricValue *mvr = fm->metricsR().get( _mid);
            if ( mvl)
                chart->addSeries( createMetricPoint( mc, mvl, "Subject (L)", Qt::darkBlue, age));
            if ( mvr)
                chart->addSeries( createMetricPoint( mc, mvr, "Subject (R)", Qt::darkGreen, age));
        }   // end if
        else
        {
            const MetricValue *mv = fm->metrics().get( _mid);
            if ( mv)
                chart->addSeries( createMetricPoint( mc, mv, "Subject", Qt::darkRed, age));
        }   // end else
    }   // end if

    chart->createDefaultAxes();

    QValueAxis* xaxis = new QValueAxis;
    xaxis->setLabelFormat( "%d");
    xaxis->setRange( xmin, xmax);
    int nyears = (int(xmax) - int(xmin));   // TODO make xtick count change on resizing of window
    xaxis->setTickCount( nyears + 1);
    chart->setAxisX( xaxis);

    chart->legend()->setAlignment(Qt::AlignRight);
    chart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    chart->setDropShadowEnabled(false);
    chart->axisX()->setTitleText( "Age");
    chart->axisY()->setTitleText( "Distance (mm)");

    QString demog;
    if ( !mc->ethnicities().isEmpty())
        demog = mc->ethnicities() + ", ";
    demog += toLongSexString( mc->sex());

    chart->setTitle( QString("<center><big><b>%1</b> (%2)</big><br><em>%3</em></center>").arg( mc->name(), demog, mc->source()));
}   // end tellReady


void ActionShowChart::purge( const FM*)
{
    resetChart( nullptr);
}   // end purge
