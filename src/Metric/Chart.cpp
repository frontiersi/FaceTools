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

#include <Metric/Chart.h>
#include <Metric/MetricManager.h>
#include <Metric/StatsManager.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
using FaceTools::Metric::Chart;
using FaceTools::Metric::GrowthData;
using FaceTools::FM;
using MM = FaceTools::Metric::MetricManager;

using namespace QtCharts;


namespace {

void updateAxisRange( QValueAxis& axis, int mi, int ma, int maxTicks=16)
{
    int t = ma - mi;   // TODO make xtick count change on resizing of window
    int shifted = 0;
    while ( t >= maxTicks)
    {
        if ( t % 2 == 0)
            t /= 2;
        else if ( t % 3 == 0)
            t /= 3;
        else if ( t % 5 == 0)
            t /= 5;
        else if ( t % 7 == 0)
            t /= 7;
        else
        {
            ma++;
            t = ma - mi;
            shifted++;
        }   // end else
    }   // end while

    if ( shifted >= 2)
    {
        const int hshift = shifted/2;   // Integer (don't care about possible remainder)
        ma -= hshift;
        mi -= hshift;
    }   // end if

    axis.setRange( mi, ma);
    axis.setTickCount( t + 1);
}   // end updateAxisRange

}   // end namespace


Chart::Ptr Chart::create( int mid, size_t d, const FM* fm)
{
    assert(MM::metric(mid) != nullptr);
    return Ptr( new Chart( mid, d, fm));
}   // end create


Chart::Chart( int mid, size_t d, const FM *fm)
    : _mid(mid), _dim(d), _gdata(nullptr),
    _ymin(FLT_MAX), _ymax(-FLT_MAX), _xmin(0), _xmax(0),
    _ageOutOfBounds(false)
{
    StatsManager::RPtr gd = StatsManager::stats( mid, fm);
    _gdata = gd.get();

    rlib::RSD::CPtr rd = _gdata->rsd(_dim);
    // The min and max age (X) range is defined by the age range of the growth data
    _xmin = int(floorf(rd->tmin()));
    _xmax = int(ceilf(rd->tmax()));

    if ( fm)
    {
        _addDataPoints( fm);
        _ageOutOfBounds = !gd->isWithinAgeRange(fm->age());
    }   // end if
    _addSeriesToChart();

    createDefaultAxes();

    QValueAxis* xaxis = static_cast<QValueAxis*>(this->axes(Qt::Horizontal).first());
    xaxis->setLabelFormat( "%d");
    xaxis->setTitleText(tr("Age (years)"));
    updateAxisRange( *xaxis, _xmin, _xmax);

    QValueAxis* yaxis = static_cast<QValueAxis*>(this->axes(Qt::Vertical).first());
    yaxis->setLabelFormat( "%d");
    MC::Ptr metric = MM::metric(mid);
    yaxis->setTitleText( tr("%1 (%2)").arg(metric->category()).arg(metric->units()));
    updateAxisRange( *yaxis, int(floorf(_ymin)), int(ceilf(_ymax)));

    this->legend()->setAlignment(Qt::AlignRight);
    this->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);

    this->setBackgroundVisible(false);
    this->setDropShadowEnabled(false);
}   // end ctor


namespace {
QString makeTitleString( const GrowthData *gd, size_t dim)
{
    QString title = MM::metric(gd->metricId())->name();
    // For multi-dimensional metrics, specify the dimension being shown.
    if ( gd->dims() > 1)
        title += QString( " [Dimension %1]").arg(dim + 1);
    return title;
}   // end makeTitleString


QString makeDemographicString( const GrowthData *gd, bool asLatex)
{
    // Get the statics info as "sex; ethnicity; N"
    const QString ethName = FaceTools::Ethnicities::name( gd->ethnicity());
    QString demog = FaceTools::toLongSexString( gd->sex());
    if ( asLatex)   // Replace possible "|" with \mid
        demog.replace("|", "$\\mid$");
    if ( !ethName.isEmpty())
        demog += "; " + ethName;
    if ( gd->n() > 0)
        demog += QString("; N=%1").arg(gd->n());
    return demog;
}   // end makeDemographicString


QString makeSourceString( const GrowthData *gd, bool ageOutOfBounds, const QString& lb)
{
    // Get the source and any note on the same line, with long notes underneath.
    QString src = gd->source();
    if ( !gd->note().isEmpty())
        src += " " + gd->note();
    if ( !gd->longNote().isEmpty())
        src += lb + gd->longNote();
    if ( ageOutOfBounds)
        src += lb + "[Age Outside Domain]";
    return src;
}   // end makeSourceString
}   // end namespace


QString Chart::makeRichTextTitleString() const
{
    const QString title = makeTitleString( _gdata, _dim);
    const QString demog = makeDemographicString( _gdata, false);
    const QString src = makeSourceString( _gdata, _ageOutOfBounds, "<br>");
    return QString("<center><big><b>%1</b> (%2)</big><br><em>%3</em></center>").arg( title, demog, src);
}   // end makeRichTextTitleString


QString Chart::makeLatexTitleString( int fnm) const
{
    const QString title = makeTitleString( _gdata, _dim);
    const QString demog = makeDemographicString( _gdata, true);
    if ( fnm > 0)
        return QString("\\textbf{%1}\\\\ \\small{(%2) \\footnotemark[%3]}").arg( title, demog).arg(fnm);
    const QString src = makeSourceString( _gdata, _ageOutOfBounds, "\\\\");
    return QString("\\textbf{%1}\\\\ \\small{(%2)}\\\\ \\scriptsize{\\textit{%3}}").arg( title, demog, src);
}   // end makeLatexTitleString


void Chart::addTitle() { this->setTitle( makeRichTextTitleString());}


namespace {
QAbstractSeries* createMetricPoint( double age, double val, const QString& title, const QColor& c, bool outBounds)
{
    QScatterSeries *dpoints = new QScatterSeries;
    dpoints->setName( title);
    //dpoints->setMarkerShape( QScatterSeries::MarkerShapeRectangle);
    dpoints->setMarkerShape( QScatterSeries::MarkerShapeCircle);
    dpoints->setMarkerSize(10);
    dpoints->setBorderColor(c);
    dpoints->setColor(Qt::white);
    if ( !outBounds)
        dpoints->setColor(c);
    dpoints->append( age, val);
    return dpoints;
}   // end createMetricPoint
}   // end namespace


void Chart::_addDataPoints( const FM* fm)
{
    float age = fm->age();
    const bool isOutOfBounds = !_gdata->isWithinAgeRange(age);
    age = std::max<float>( _xmin, std::min<float>( age, _xmax));

    FaceAssessment::CPtr ass = fm->currentAssessment();
    if ( MM::metric(_mid)->isBilateral())
    {
        if ( ass->cmetrics(LEFT).has(_mid))
        {
            assert( ass->cmetrics(RIGHT).has(_mid));
            const MetricValue& mvl = ass->cmetrics(LEFT).metric( _mid);
            const MetricValue& mvr = ass->cmetrics(RIGHT).metric( _mid);
            const float val = 0.5f * (mvl.value(_dim) + mvr.value(_dim));

            this->addSeries( createMetricPoint( age, mvl.value(_dim), "Left", Qt::blue, isOutOfBounds));
            this->addSeries( createMetricPoint( age, mvr.value(_dim), "Right", Qt::green, isOutOfBounds));
            this->addSeries( createMetricPoint( age, val, "Mean", Qt::red, isOutOfBounds));

            _ymin = std::min( mvl.value(_dim), mvr.value(_dim));
            _ymax = std::max( mvl.value(_dim), mvr.value(_dim));
        }   // end if
    }   // end if
    else if ( ass->cmetrics(MID).has(_mid))
    {
        const MetricValue &mv = ass->cmetrics(MID).metric( _mid);
        this->addSeries( createMetricPoint( age, mv.value(_dim), "Subject", Qt::red, isOutOfBounds));
        _ymin = _ymax = mv.value(_dim);
    }   // end else if
}   // end _addDataPoints


void Chart::_addSeriesToChart()
{
    QLineSeries *mseries = new QSplineSeries;
    QLineSeries *z1pseries = new QSplineSeries;
    QLineSeries *z2pseries = new QSplineSeries;
    QLineSeries *z1nseries = new QSplineSeries;
    QLineSeries *z2nseries = new QSplineSeries;

    assert( _gdata);
    rlib::RSD::CPtr rd = _gdata->rsd(_dim);
    for ( int i = _xmin; i <= _xmax; ++i)
    {
        float a = i;
        float m = rd->mval(a);
        float z = rd->zval(a);

        mseries->append( a, m);
        z1pseries->append( a, m + z);
        z2pseries->append( a, m + 2*z);
        z1nseries->append( a, m - z);
        z2nseries->append( a, m - 2*z);

        _ymin = std::min( _ymin, m - 2*z);
        _ymax = std::max( _ymax, m + 2*z);
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
}   // end _addSeriesToChart
