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

#include <Chart.h>
#include <FaceModel.h>
#include <Ethnicities.h>
#include <MetricCalculatorManager.h>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
using FaceTools::Metric::Chart;
using FaceTools::Metric::GrowthData;
using FaceTools::FM;
using MCM = FaceTools::Metric::MetricCalculatorManager;

using namespace QtCharts;


namespace {

QScatterSeries* createMetricPoint( double age, double val, const QString& title, const QColor& c)
{
    QScatterSeries *dpoints = new QScatterSeries;
    dpoints->setName( title);
    dpoints->setMarkerShape( QScatterSeries::MarkerShapeCircle);
    dpoints->setMarkerSize(10);
    dpoints->setColor(c);
    dpoints->append( age, val);
    return dpoints;
}   // end createMetricPoint


void updateXAxisRange( QValueAxis& xaxis, double xmin, double xmax)
{
    xaxis.setRange( xmin, xmax);
    int nyears = (int(xmax) - int(xmin));   // TODO make xtick count change on resizing of window
    int tc = nyears + 1;
    if ( nyears > 20)
        tc /= 2;
    xaxis.setTickCount( tc);
}   // end updateXAxisRange

}   // end namespace


Chart::Ptr Chart::create( GrowthData::CPtr gd, size_t d, const FM* fm)
{
    return Ptr( new Chart( gd, d, fm));
}   // end create


Chart::Chart( GrowthData::CPtr gd, size_t d, const FM* fm) : _gdata(gd), _dim(d)
{
    double xmin, xmax;
    _addSeriesToChart( xmin, xmax);
    if ( fm)
    {
        const double age = fm->age();
        xmin = std::min<double>( xmin, age);
        xmax = std::max<double>( xmax, age);
        _addDataPoints( fm);
    }   // end if

    this->createDefaultAxes();

    QValueAxis* xaxis = new QValueAxis;
    xaxis->setLabelFormat( "%d");
    updateXAxisRange( *xaxis, xmin, xmax);

    this->setAxisX( xaxis);

    this->legend()->setAlignment(Qt::AlignRight);
    this->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    this->setBackgroundVisible(false);
    this->setDropShadowEnabled(false);
    this->axisX()->setTitleText( "Age from birth (years)");
    this->axisY()->setTitleText( QString("Distance (%1)").arg(FM::LENGTH_UNITS));
}   // end ctor


Chart::~Chart() { }   // end dtor


namespace {
QString makeTitleString( GrowthData::CPtr gd, size_t dim)
{
    QString title = MCM::metric(gd->metricId())->name();
    // For multi-dimensional metrics, specify the dimension being shown.
    if ( gd->dims() > 1)
        title += QString( " [Dimension %1]").arg(dim + 1);
    return title;
}   // end makeTitleString


QString makeDemographicString( GrowthData::CPtr gd)
{
    // Get the statics info as "sex; ethnicity; N"
    const QString ethName = FaceTools::Ethnicities::name( gd->ethnicity());
    QString demog = FaceTools::toLongSexString( gd->sex());
    if ( !ethName.isEmpty())
        demog += "; " + ethName;
    if ( gd->n() > 0)
        demog += QString("; N=%1").arg(gd->n());
    return demog;
}   // end makeDemographicString


QString makeSourceString( GrowthData::CPtr gd, const QString& lb)
{
    // Get the source and any note on the same line, with long notes underneath.
    QString src = gd->source();
    if ( !gd->note().isEmpty())
        src += " " + gd->note();
    if ( !gd->longNote().isEmpty())
        src += lb + gd->longNote();
    return src;
}   // end makeSourceString
}   // end namespace


QString Chart::makeRichTextTitleString() const
{
    const QString title = makeTitleString( _gdata, _dim);
    const QString demog = makeDemographicString( _gdata);
    const QString src = makeSourceString( _gdata, "<br>");
    // Format as centred:
    // <big><b>Title</b> (Sex; Ethnicity; N=n)</big>
    //           <em>Source Note
    //               LongNote</em>
    return QString("<center><big><b>%1</b> (%2)</big><br><em>%3</em></center>").arg( title, demog, src);
}   // end makeRichTextTitleString


QString Chart::makeLatexTitleString( int fnm) const
{
    const QString title = makeTitleString( _gdata, _dim);
    const QString demog = makeDemographicString( _gdata);
    if ( fnm > 0)
        return QString("\\textbf{%1}\\\\ \\small{(%2) \\footnotemark[%3]}").arg( title, demog).arg(fnm);
    const QString src = makeSourceString( _gdata, "\\\\");
    return QString("\\textbf{%1}\\\\ \\small{(%2)}\\\\ \\scriptsize{\\textit{%3}}").arg( title, demog, src);
}   // end makeLatexTitleString


void Chart::addTitle() { this->setTitle( makeRichTextTitleString());}


void Chart::_addDataPoints( const FM* fm)
{
    GrowthData::CPtr gd = _gdata;
    const int mid = gd->metricId();
    const double age = fm->age();
    FaceAssessment::CPtr ass = fm->currentAssessment();

    if ( MCM::metric(mid)->isBilateral())
    {
        if ( ass->cmetricsL().has(mid) && ass->cmetricsR().has(mid))
        {
            const MetricValue& mvl = ass->cmetricsL().metric( mid);
            const MetricValue& mvr = ass->cmetricsR().metric( mid);

            this->addSeries( createMetricPoint( age, mvl.value(_dim), "Left", Qt::blue));
            this->addSeries( createMetricPoint( age, mvr.value(_dim), "Right", Qt::darkGreen));

            const double val = 0.5 * (mvl.value(_dim) + mvr.value(_dim));
            this->addSeries( createMetricPoint( age, val, "Mean", Qt::red));
        }   // end if
    }   // end if
    else
    {
        if ( ass->cmetrics().has(mid))
        {
            const MetricValue &mv = ass->cmetrics().metric( mid);
            this->addSeries( createMetricPoint( age, mv.value(_dim), "Subject", Qt::red));
        }   // end if
    }   // end else
}   // end _addDataPoints


void Chart::_addSeriesToChart( double &x0, double &x1)
{
    x0 = DBL_MAX;
    x1 = -DBL_MAX;

    QSplineSeries *mseries = new QSplineSeries;
    QSplineSeries *z1pseries = new QSplineSeries;
    QSplineSeries *z2pseries = new QSplineSeries;
    QSplineSeries *z1nseries = new QSplineSeries;
    QSplineSeries *z2nseries = new QSplineSeries;

    assert( _gdata);
    rlib::RSD::CPtr rd = _gdata->rsd(_dim);
    const int minx = int(rd->tmin());
    const int maxx = int(rd->tmax() + 0.5);

    for ( int i = minx; i <= maxx; ++i)
    {
        double a = i;
        double m = rd->mval(a);
        double z = rd->zval(a);
        mseries->append( a, m);
        z1pseries->append( a, m + z);
        z2pseries->append( a, m + 2*z);
        z1nseries->append( a, m - z);
        z2nseries->append( a, m - 2*z);
    }   // end for

    z2pseries->setName("+2SD");
    z1pseries->setName("+1SD");
    mseries->setName("Mean");
    z1nseries->setName("-1SD");
    z2nseries->setName("-2SD");

    QPen pen;
    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    pen.setColor(Qt::red);
    z2pseries->setPen(pen);
    pen.setColor(Qt::darkRed);
    z1pseries->setPen(pen);

    pen.setColor(Qt::blue);
    z2nseries->setPen(pen);
    pen.setColor(Qt::darkBlue);
    z1nseries->setPen(pen);

    pen.setStyle(Qt::SolidLine);
    pen.setColor(Qt::black);
    mseries->setPen(pen);

    addSeries(z2pseries);
    addSeries(z1pseries);
    addSeries(mseries);
    addSeries(z1nseries);
    addSeries(z2nseries);

    x0 = std::min<double>( x0, minx);
    x1 = std::max<double>( x1, maxx);
}   // end _addSeriesToChart
