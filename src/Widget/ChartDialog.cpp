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

#include <ChartDialog.h>
#include <ui_ChartDialog.h>
#include <MetricCalculatorManager.h>
#include <ModelSelector.h>
#include <FaceModel.h>
#include <QPushButton>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QValueAxis>
#include <cassert>
#include <cmath>
using FaceTools::Widget::ChartDialog;
using FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MC;
using FaceTools::FM;
using FaceTools::Sex;
using FaceTools::Action::ModelSelector;
using MCM = FaceTools::Metric::MetricCalculatorManager;


ChartDialog::ChartDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ChartDialog), _cview( new QtCharts::QChartView), _ignEthn(false)
{
    _ui->setupUi(this);
    _ui->frame->layout()->addWidget( _cview);

    _cview->setRenderHint( QPainter::Antialiasing);
    setWindowTitle( parent->windowTitle() + " | Metric Growth Chart");
    resize( 850, 500);

    connect( _ui->sourceComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnUserSelectedSource);
    connect( _ui->ethnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnUserSelectedEthnicity);
    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnResetChart);
    connect( _ui->dimensionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartDialog::doOnResetChart);
    connect( _ui->editDataButton, &QPushButton::clicked, this, &ChartDialog::doOnEditData);

    assert( MCM::count() > 0);
    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::activated, this, &ChartDialog::doOnSetMetricActive);
    doOnSetMetricActive();
}   // end ctor


ChartDialog::~ChartDialog() { delete _ui;}


void ChartDialog::refresh()
{
    if ( ModelSelector::selected())
    {
        const FM* fm = ModelSelector::selected()->data();
        const QString& e = fm->ethnicity();
        const int8_t s = fm->sex();

        MC::Ptr mc = MCM::activeMetric();
        if ( mc && mc->ethnicities().count(e) > 0)
        {
            _ui->ethnicityComboBox->setCurrentText(e);
            if ( mc->growthData(e, s))
                _ui->sexComboBox->setCurrentText( toLongSexString(s));
        }   // end if
    }   // end if

    doOnResetChart();
}   // end setDemographic


void ChartDialog::doOnSetEthnicityIgnored(bool v)
{
    _ignEthn = v;
    refresh();
}   // end doOnSetEthnicityIgnored


// private
void ChartDialog::doOnSetMetricActive()
{
    populateSources();
    populateEthnicites();
    populateSexs();
    populateDimensions();
    refresh();
}   // end doOnSetMetricActive


// private
void ChartDialog::populateSources()
{
    _ui->sourceComboBox->clear();
    MC::Ptr mc = MCM::activeMetric();
    assert(mc);
    QStringList slst;
    for ( const QString& s : mc->sources())
        slst.append(s);

    slst.sort();
    for ( const QString& s : slst)
        _ui->sourceComboBox->addItem(s);

    _ui->sourceComboBox->setCurrentText( mc->source());
    _ui->sourceComboBox->setEnabled( _ui->sourceComboBox->count() > 1);
}   // end populateSources


// private
void ChartDialog::populateEthnicites()
{
    _ui->ethnicityComboBox->clear();
    MC::Ptr mc = MCM::activeMetric();
    assert(mc);

    const QString src = _ui->sourceComboBox->currentText();
    mc->setSource(src);

    QStringList elst;
    for ( const QString& e : mc->ethnicities())
        elst.append(e);

    elst.sort();
    for ( const QString& e : elst)
        _ui->ethnicityComboBox->addItem(e);

    _ui->ethnicityComboBox->setEnabled( _ui->ethnicityComboBox->count() > 1);
}   // end populateEthnicity


// private
void ChartDialog::populateSexs()
{
    _ui->sexComboBox->clear();
    MC::Ptr mc = MCM::activeMetric();
    assert(mc);

    const QString ethn = _ui->ethnicityComboBox->currentText();
    if ( mc->growthData( ethn, int8_t(FEMALE_SEX | MALE_SEX)))
        _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX | MALE_SEX));
    else
    {
        if ( mc->growthData( ethn, FEMALE_SEX))
            _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX));
        if ( mc->growthData( ethn, MALE_SEX))
            _ui->sexComboBox->addItem( toLongSexString( MALE_SEX));
    }   // end else

    _ui->sexComboBox->setEnabled( _ui->sexComboBox->count() > 1);
}   // end populateSex


// private
void ChartDialog::populateDimensions()
{
    const int ndims = static_cast<int>( MCM::activeMetric()->dims());
    _ui->dimensionSpinBox->setMaximum( ndims);
    _ui->dimensionSpinBox->setValue(1);
    _ui->dimensionSpinBox->setEnabled( ndims > 1);
}   // end populateDimensions


// private
const FM* ChartDialog::isDemographic() const
{
    if ( !ModelSelector::selected())
        return nullptr;

    const FM* fm = ModelSelector::selected()->data();
    const int8_t ssex = fromLongSexString( _ui->sexComboBox->currentText());
    // Ethnicity must match exactly unless ignoring ethnicity
    const bool ethnicityMatch = _ignEthn || (fm->ethnicity() == _ui->ethnicityComboBox->currentText());
    const bool match = ethnicityMatch && (( ssex == int8_t(FEMALE_SEX | MALE_SEX)) || ( fm->sex() == ssex));
    return match ? fm : nullptr;
}   // end isDemographic


// private
void ChartDialog::doOnEditData()
{
    ; // TODO
}   // end doOnEditData


// private
void ChartDialog::doOnUserSelectedSource()
{
    populateEthnicites();
    doOnUserSelectedEthnicity();
}   // end doOnUserSelectedSource


// private
void ChartDialog::doOnUserSelectedEthnicity()
{
    populateSexs();
    doOnResetChart();
}   // end doOnUserSelectedEthnicity


namespace {

QtCharts::QScatterSeries* createMetricPoint( double age, double val, const QString& title, const QColor& c)
{
    using namespace QtCharts;
    QScatterSeries *dpoints = new QScatterSeries;
    dpoints->setName( title);
    dpoints->setMarkerShape(QScatterSeries::MarkerShapeCircle);
    dpoints->setMarkerSize(10);
    dpoints->setColor(c);
    dpoints->append( age, val);
    return dpoints;
}   // end createMetricPoint

}   // end namespace


// private
void ChartDialog::doOnResetChart()
{
    using namespace QtCharts;
    QChart *chart = new QChart;
    _cview->setChart(chart);

    QString ethn = _ui->ethnicityComboBox->currentText();
    const int8_t sex = fromLongSexString(_ui->sexComboBox->currentText());

    const size_t d = static_cast<size_t>(_ui->dimensionSpinBox->value() - 1);   // The selected dimension to display
    assert( d >= 0);

    MC::Ptr mc = MCM::activeMetric();
    assert(mc);

    const GrowthData* gd = mc->growthData( ethn, sex);
    if ( gd)
    {
        double xmin = 0;
        double xmax = 0;
        mc->addSeriesToChart( chart, gd, &xmin, &xmax);

        const FM* fm;
        if ( (fm = isDemographic()))
        {
            const double age = fm->age();
            xmin = std::min<double>(xmin, age);
            xmax = std::max<double>(xmax, age);

            if ( mc->isBilateral())
            {
                const MetricValue *mvl = fm->metricsL().get( mc->id());
                const MetricValue *mvr = fm->metricsR().get( mc->id());

                if ( mvl)
                    chart->addSeries( createMetricPoint( age, mvl->value(d), "Subject (Left)", Qt::blue));

                if ( mvl && mvr) // Create a mean data point
                {
                    double val = 0.5 * (mvl->value(d) + mvr->value(d));
                    chart->addSeries( createMetricPoint( age, val, "Subject (Mean)", Qt::red));
                }   // end if

                if ( mvr)
                    chart->addSeries( createMetricPoint( age, mvr->value(d), "Subject (Right)", Qt::darkGreen));
            }   // end if
            else
            {
                const MetricValue *mv = fm->metrics().get( mc->id());
                if ( mv)
                    chart->addSeries( createMetricPoint( age, mv->value(d), "Subject", Qt::red));
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
        chart->axisX()->setTitleText( "Age (yrs)");
        chart->axisY()->setTitleText( QString("Distance (%1)").arg(FM::LENGTH_UNITS));
    }   // end if

    QString title = mc->name();
    if ( mc->dims() > 1)    // Specify which dimension of the metric is being shown
        title += QString( "(Dimension %1)").arg(d);

    QString demog, src;
    if ( !gd)
        demog = "No growth curve data!";
    else
    {
        if ( !gd->ethnicity().isEmpty())
            demog = gd->ethnicity() + "; ";
        demog += toLongSexString( static_cast<Sex>(gd->sex()));

        src = "<em>" + gd->source();
        if ( !gd->note().isEmpty())
            src += "<br>" + gd->note();
        if ( gd->n() > 0)
            src += QString("; N=%1").arg(gd->n());
        src += "</em>";
    }   // end else

    chart->setTitle( QString("<center><big><b>%1</b> (%2)</big><br>%3</center>").arg( title, demog, src));
}   // end doOnResetChart
