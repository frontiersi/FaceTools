/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <Widget/ChartDialog.h>
#include <ui_ChartDialog.h>
#include <Metric/MetricCalculatorManager.h>
#include <Action/ModelSelector.h>
#include <Metric/Chart.h>
#include <Ethnicities.h>
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
using FaceTools::Metric::GrowthData;
using GDS = FaceTools::Metric::GrowthDataSources;
using FaceTools::Metric::MC;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using MS = FaceTools::Action::ModelSelector;


ChartDialog::ChartDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ChartDialog),
    _cview( new QtCharts::QChartView)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Metric Growth Curves");

    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->frame->layout()->addWidget( _cview);
    _cview->setRenderHint( QPainter::Antialiasing);

    assert( MCM::count() > 0);

    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::_doOnUserSelectedSex);
    connect( _ui->ethnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::_doOnUserSelectedEthnicity);
    connect( _ui->sourceComboBox, QOverload<int>::of(&QComboBox::activated), this, &ChartDialog::_updateChart);
    connect( _ui->dimensionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartDialog::_updateChart);
    connect( _ui->saveImageButton, &QToolButton::clicked, this, &ChartDialog::_doOnSaveImage);
}   // end ctor


ChartDialog::~ChartDialog() { delete _ui;}


void ChartDialog::_populateSexs( int ethn)
{
    _ui->sexComboBox->clear();
    _ui->sexComboBox->setEnabled( false);

    const GDS& gds = MCM::currentMetric()->compatibleSources();
    std::unordered_set<int8_t> sexs;
    for ( GrowthData::CPtr gd : gds)
    {
        if ( ethn == 0 || gd->ethnicity() == ethn)
        {
            sexs.insert(gd->sex());
            if ( sexs.size() == 3)
                break;
        }   // end if
    }   // end for

    if ( sexs.count(FEMALE_SEX | MALE_SEX) > 0)
        _ui->sexComboBox->addItem( toLongSexString(FEMALE_SEX | MALE_SEX), FEMALE_SEX | MALE_SEX);
    if ( sexs.count(FEMALE_SEX) > 0)
        _ui->sexComboBox->addItem( toLongSexString(FEMALE_SEX), FEMALE_SEX);
    if ( sexs.count(MALE_SEX) > 0)
        _ui->sexComboBox->addItem( toLongSexString(MALE_SEX), MALE_SEX);

    _ui->sexComboBox->setEnabled( _ui->sexComboBox->count() > 1);
}   // end _populateSexs


void ChartDialog::_populateEthnicities( int8_t sex)
{
    _ui->ethnicityComboBox->clear();
    _ui->ethnicityComboBox->setEnabled(false);

    const GDS& gds = MCM::currentMetric()->compatibleSources();
    std::unordered_set<int> eset;
    for ( GrowthData::CPtr gd : gds)
    {
        if ( sex == UNKNOWN_SEX || gd->sex() == sex)
            eset.insert( gd->ethnicity());
    }   // end for

    QStringList elst;
    std::unordered_map<QString, int> emap;
    for ( int e : eset)
    {
        const QString& en = Ethnicities::name(e);
        if ( emap.count(en) == 0)
        {
            emap[en] = e;
            elst << en;
        }   // end if
    }   // end for

    elst.sort();
    for ( const QString& en : elst)
        _ui->ethnicityComboBox->addItem( en, emap[en]);
    _ui->ethnicityComboBox->setEnabled(  _ui->ethnicityComboBox->count() > 1);
}   // end _populateEthnicities


void ChartDialog::_doOnUserSelectedSex()
{
    MC::Ptr mc = MCM::currentMetric();
    _populateEthnicities( _selectedSex());
    _setEthnicity();
    _populateSources();
    _updateChart();
}   // end _doOnUserSelectedSex


void ChartDialog::_doOnUserSelectedEthnicity()
{
    MC::Ptr mc = MCM::currentMetric();
    _populateSexs( _selectedEthnicity());
    _setSex();
    _populateSources();
    _updateChart();
}   // end _doOnUserSelectedEthnicity


void ChartDialog::refreshMetricOrModel()
{
    _populateDimensions();
    MC::Ptr mc = MCM::currentMetric();
    assert(mc);
    _populateSexs( UNKNOWN_SEX);
    _populateEthnicities( 0);
    _setSex();
    _setEthnicity();
    _populateSources();
    _populateSources();
    _updateChart();
}   // end refreshMetricOrModel


void ChartDialog::_populateSources()
{
    _ui->sourceComboBox->clear();
    _ui->sourceComboBox->setEnabled(false);
    MC::Ptr mc = MCM::currentMetric();
    assert(mc);

    GDS gds = mc->matchingGrowthData( _selectedSex(), _selectedEthnicity(), true);
    QStringList slst;
    for ( GrowthData::CPtr gd : gds)
        slst.append( gd->source());
    slst.sort();
    _ui->sourceComboBox->addItems(slst);

    if ( _ui->sourceComboBox->count() > 0)
    {
        int idx = 0;
        // Use the metric's current source if possible.
        if ( mc->currentGrowthData() != nullptr)
        {
            idx = _ui->sourceComboBox->findText( mc->currentGrowthData()->source());
            if ( idx < 0)
                idx = 0;
        }   // end if
        _ui->sourceComboBox->setCurrentIndex(idx);
    }   // end if

    _ui->sourceComboBox->setEnabled( _ui->sourceComboBox->count() > 1);
}   // end _populateSources


void ChartDialog::_populateDimensions()
{
    MC::Ptr mc = MCM::currentMetric();
    const int ndims = static_cast<int>( mc->dims());
    _ui->dimensionSpinBox->setMaximum( ndims);
    _ui->dimensionSpinBox->setValue(1);
    _ui->dimensionSpinBox->setEnabled( mc->currentGrowthData() && ndims > 1);
}   // end _populateDimensions


int8_t ChartDialog::_selectedSex() const { return int8_t( _ui->sexComboBox->currentData().toInt() & 0xff);}
int ChartDialog::_selectedEthnicity() const { return _ui->ethnicityComboBox->currentData().toInt();}
QString ChartDialog::_selectedSource() const { return _ui->sourceComboBox->currentText();}


// Set ethnicity to the current growth data if possible or the top of the list if not found.
void ChartDialog::_setEthnicity()
{
    MC::Ptr mc = MCM::currentMetric();
    const int ethn = mc && mc->currentGrowthData() ? mc->currentGrowthData()->ethnicity() : 0;
    if ( _ui->ethnicityComboBox->count() > 0)
        _ui->ethnicityComboBox->setCurrentIndex( std::max( 0, _ui->ethnicityComboBox->findData( ethn)));
}   // end _setEthnicity


// Set sex to the current growth data if possible or the top of the list if not found.
void ChartDialog::_setSex()
{
    MC::Ptr mc = MCM::currentMetric();
    const int8_t sex = mc && mc->currentGrowthData() ? mc->currentGrowthData()->sex() : UNKNOWN_SEX;
    if ( _ui->sexComboBox->count() > 0)
        _ui->sexComboBox->setCurrentIndex( std::max( 0, _ui->sexComboBox->findData( sex)));
}   // end _setSex


void ChartDialog::_updateChart()
{
    MC::Ptr mc = MCM::currentMetric();
    GrowthData::CPtr ngd = mc->growthData( _selectedSex(), _selectedEthnicity(), _selectedSource());

    if ( ngd)
    {
        const size_t d = static_cast<size_t>(_ui->dimensionSpinBox->value() - 1);   // Dimension to display
        Metric::Chart* chart = new Metric::Chart( ngd, d, MS::selectedModel());
        chart->addTitle();
        _cview->setChart( chart);
    }   // end if
    else
    {
        static const QString src0 = "No growth curve data available.";
        QString src = src0;
        QtCharts::QChart* chart = new QtCharts::QChart;
        chart->setTitle( QString("<center><big><b>%1</b><br><em>%2</em></big></center>").arg( mc->name(), src));
        _cview->setChart(chart);
    }   // end else

    _ui->saveImageButton->setEnabled( ngd != nullptr);

    if ( mc->currentGrowthData() != ngd)
    {
        mc->setCurrentGrowthData( ngd);
        emit onGrowthDataChanged();
    }   // end if
}   // end _updateChart


void ChartDialog::_doOnSaveImage()
{
    QWidget* prnt = static_cast<QWidget*>(parent());
    QFileDialog fdialog( prnt);
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
        QMessageBox::warning( prnt, tr("Save Error!"), tr("Unable to save to file ") + imgpath);
}   // end _doOnSaveImage
