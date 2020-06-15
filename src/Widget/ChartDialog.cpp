/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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
#include <Metric/MetricManager.h>
#include <Action/ModelSelector.h>
#include <Metric/Chart.h>
#include <QMessageBox>
#include <QSvgGenerator>
using FaceTools::Widget::ChartDialog;
using FaceTools::Metric::GrowthData;
using FaceTools::Metric::MC;
using MM = FaceTools::Metric::MetricManager;
using MS = FaceTools::Action::ModelSelector;


ChartDialog::ChartDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ChartDialog), _cview( new QtCharts::QChartView), _fdialog(nullptr)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Metric Growth Curves");

    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->frame->layout()->addWidget( _cview);
    _cview->setRenderHint( QPainter::Antialiasing);

    connect( _ui->saveButton, &QToolButton::clicked, this, &ChartDialog::_doOnSaveImage);
    connect( _ui->dimensionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ChartDialog::_doRefreshChart);

    _fdialog = new QFileDialog( this);
    _fdialog->setWindowTitle( tr("Save image as..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setViewMode( QFileDialog::Detail);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
    _fdialog->setNameFilter( tr("Image Files (*.png *.jpg *.jpeg *.gif *.bmp *.svg)"));
}   // end ctor


ChartDialog::~ChartDialog() { delete _ui;}


void ChartDialog::refresh()
{
    _refreshDimensions();
    _doRefreshChart();
}   // end refresh


void ChartDialog::_refreshDimensions()
{
    MC::Ptr mc = MM::currentMetric();
    const int ndims = mc ? static_cast<int>( mc->dims()) : 1;
    _ui->dimensionSpinBox->setMaximum( ndims);
    _ui->dimensionSpinBox->setValue(1);
    _ui->dimensionSpinBox->setEnabled( ndims > 1 && mc->growthData().current());
}   // end _refreshDimensions


void ChartDialog::_doRefreshChart()
{
    MC::Ptr mc = MM::currentMetric();
    const GrowthData *gd = mc ? mc->growthData().current() : nullptr;

    if ( gd)
    {
        const size_t d = static_cast<size_t>(_ui->dimensionSpinBox->value() - 1);   // Dimension to display
        Metric::Chart* chart = new Metric::Chart( gd, d, MS::selectedModel());
        chart->addTitle();
        _cview->setChart( chart);
    }   // end if
    else
    {
        static const QString src0 = "Growth curve statistics N/A";
        QString src = src0;
        QtCharts::QChart* chart = new QtCharts::QChart;
        QString mname = mc ? mc->name() : "No current measurement";
        chart->setTitle( QString("<center><big><b>%1</b><br><em>%2</em></big></center>").arg( mname, src));
        _cview->setChart(chart);
    }   // end else

    _ui->saveButton->setEnabled( gd != nullptr);
}   // end _doRefreshChart


namespace {
bool hasValidExtension( const QString& fname)
{
    return fname.endsWith( ".jpg", Qt::CaseInsensitive)
        || fname.endsWith( ".jpeg", Qt::CaseInsensitive)
        || fname.endsWith( ".png", Qt::CaseInsensitive)
        || fname.endsWith( ".gif", Qt::CaseInsensitive)
        || fname.endsWith( ".bmp", Qt::CaseInsensitive);
}   // end hasValidExtension
}   // end namespace


void ChartDialog::_doOnSaveImage()
{
    _fdialog->selectFile( "chart.png");
    QString imgpath;
    if ( _fdialog->exec())
        imgpath = _fdialog->selectedFiles().first().trimmed();

    if ( !imgpath.isEmpty())
    {
        bool savedOkay = true;

        if ( QFileInfo(imgpath).suffix().toLower() == "svg")
        {
            QSvgGenerator simg;
            simg.setFileName( imgpath);
            simg.setSize( _cview->size());
            simg.setViewBox( QRect( 0, 0, _cview->width(), _cview->height()));
            static_cast<QWidget*>(_cview)->render( &simg);
        }   // end if
        else
        {
            if ( !hasValidExtension(imgpath))
                imgpath += ".jpg";

            QPixmap pixmap( _cview->size());
            static_cast<QWidget*>(_cview)->render( &pixmap);
            savedOkay = pixmap.save( imgpath);
        }   // end else

        if ( !savedOkay)
            QMessageBox::warning( this, tr("Save Error!"), tr("Unable to save to file ") + imgpath);
    }   // end if
}   // end _doOnSaveImage
