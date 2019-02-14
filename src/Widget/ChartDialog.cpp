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
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
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

    assert( MCM::count() > 0);
    doOnSetSelectedMetric();

    connect( _ui->sourceComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnUserSelectedSource);
    connect( _ui->ethnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnUserSelectedEthnicity);
    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::doOnResetChart);
    connect( _ui->dimensionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartDialog::doOnResetChart);
    //connect( _ui->editDataButton, &QPushButton::clicked, this, &ChartDialog::doOnEditData);

    connect( _ui->saveImageButton, &QToolButton::clicked, this, &ChartDialog::doOnSaveImage);

    for ( MC::Ptr mc : MCM::metrics())
        connect( &*mc, &MC::selected, this, &ChartDialog::doOnSetSelectedMetric);
}   // end ctor


ChartDialog::~ChartDialog() { delete _ui;}


void ChartDialog::refresh()
{
    if ( ModelSelector::selected())
    {
        const FM* fm = ModelSelector::selected()->data();
        const QString& e = fm->ethnicity();
        const int8_t s = fm->sex();

        MC::Ptr mc = MCM::currentMetric();
        if ( mc && mc->ethnicities().count(e) > 0)
            _ui->ethnicityComboBox->setCurrentText(e);
        if ( mc->growthData(e, s))
            _ui->sexComboBox->setCurrentText( toLongSexString(s));
    }   // end if

    doOnResetChart();
}   // end setDemographic


void ChartDialog::doOnSetEthnicityIgnored(bool v)
{
    _ignEthn = v;
    refresh();
}   // end doOnSetEthnicityIgnored


// private
void ChartDialog::doOnSetSelectedMetric()
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
    MC::Ptr mc = MCM::currentMetric();
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
    MC::Ptr mc = MCM::currentMetric();
    assert(mc);

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
    MC::Ptr mc = MCM::currentMetric();
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
}   // end populateSexs


// private
void ChartDialog::populateDimensions()
{
    const int ndims = static_cast<int>( MCM::currentMetric()->dims());
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
void ChartDialog::doOnUserSelectedSource()
{
    MC::Ptr mc = MCM::currentMetric();
    assert(mc);
    const QString src = _ui->sourceComboBox->currentText();
    mc->setSource(src);
    mc->signalUpdated();

    populateEthnicites();
    doOnUserSelectedEthnicity();
}   // end doOnUserSelectedSource


// private
void ChartDialog::doOnUserSelectedEthnicity()
{
    populateSexs();
    refresh();
}   // end doOnUserSelectedEthnicity


// private
void ChartDialog::doOnResetChart()
{
    const QString ethn = _ui->ethnicityComboBox->currentText();
    const int8_t sex = fromLongSexString(_ui->sexComboBox->currentText());
    const size_t d = static_cast<size_t>(_ui->dimensionSpinBox->value() - 1);   // Dimension to display
    const MC::Ptr mc = MCM::currentMetric();
    assert( mc);
    const int mid = mc->id();
    const FM* fm = isDemographic();

    QtCharts::QChart *chart = nullptr;
    if ( !(chart = FaceTools::createChart( ethn, sex, d, mid, fm)))
    {
        chart = new QtCharts::QChart;

        QString title = mc->name();
        if ( mc->dims() > 1)    // Specify which dimension of the metric is being shown
            title += QString( " (Dimension %1)").arg(d);
        QString src = "<big><em>No growth curve data available.</em></big>";
        chart->setTitle( QString("<center><big><b>%1</b></big><br>%2</center>").arg( title, src));
    }   // end if

    _cview->setChart(chart);
}   // end doOnResetChart


// private
void ChartDialog::doOnSaveImage()
{
    QFileDialog fdialog( (QWidget*)parent());
    fdialog.setWhatsThis( tr("Save image as..."));
    fdialog.setFileMode( QFileDialog::AnyFile);
    fdialog.setNameFilter( "Image Files (*.jpg *.jpeg *.png *.gif *.bmp)");
    fdialog.selectFile( "chart.png");
    fdialog.setAcceptMode( QFileDialog::AcceptSave);
    fdialog.setOption( QFileDialog::DontUseNativeDialog);

    QStringList fnames;
    if ( fdialog.exec())
        fnames = fdialog.selectedFiles();

    QString imgpath;
    if ( !fnames.empty())
    {
        imgpath = fnames.first();
        if ( QFile::exists(imgpath))
            QFile::remove(imgpath);
    }   // end if

    QPixmap pixmap( _cview->size());
    static_cast<QWidget*>(_cview)->render( &pixmap);

    if ( !imgpath.isEmpty() && !pixmap.save(imgpath))
        QMessageBox::warning( (QWidget*)parent(), tr("Save Error!"), tr("Unable to save to file ") + imgpath);
}   // end doOnSaveImage
