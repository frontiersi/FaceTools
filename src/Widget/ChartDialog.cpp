/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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
#include <ModelSelect.h>
#include <Metric/Metric.h>
#include <Metric/Chart.h>
#include <QBuffer>
#include <QSaveFile>
#include <QMessageBox>
#include <QSvgGenerator>
#include <QSvgRenderer>
using FaceTools::Widget::ChartDialog;
using FaceTools::Metric::GrowthData;
using MC = FaceTools::Metric::Metric;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;


ChartDialog::ChartDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ChartDialog), _cview( new QtCharts::QChartView),
    _fdialog(nullptr), _metric(nullptr)
{
    _ui->setupUi(this);
    setWindowTitle( parent->windowTitle() + " | Metric Growth Curves");

    //setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    //setParent( parent, windowFlags() & ~Qt::WindowStaysOnTopHint);

    _ui->frame->layout()->addWidget( _cview);
    _cview->setRenderHint( QPainter::Antialiasing);

    connect( _ui->saveButton, &QToolButton::clicked, this, &ChartDialog::_doOnSaveImage);
    connect( _ui->dimensionSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                              this, &ChartDialog::_doRefreshChart);

    _fdialog = new QFileDialog( this);
    _fdialog->setWindowTitle( tr("Save image as..."));
    _fdialog->setFileMode( QFileDialog::AnyFile);
    _fdialog->setViewMode( QFileDialog::Detail);
    _fdialog->setAcceptMode( QFileDialog::AcceptSave);
    //_fdialog->setOption( QFileDialog::DontUseNativeDialog);
    _fdialog->setNameFilter( tr("Image Files (*.png *.jpg *.jpeg *.bmp *.svg)"));
}   // end ctor


ChartDialog::~ChartDialog() { delete _ui;}


void ChartDialog::refresh( const MC *m)
{
    _metric = m;
    _refreshDimensions();
    _doRefreshChart();
}   // end refresh


void ChartDialog::_refreshDimensions()
{
    const int ndims = _metric ? static_cast<int>( _metric->dims()) : 1;
    _ui->dimensionSpinBox->setMaximum( ndims);
    _ui->dimensionSpinBox->setValue(1);
    _ui->dimensionSpinBox->setEnabled( ndims > 1 && !_metric->growthData().all().empty());
}   // end _refreshDimensions


void ChartDialog::_doRefreshChart()
{
    const bool hasMC = _metric && !_metric->growthData().all().empty();

    if ( hasMC)
    {
        const size_t d = static_cast<size_t>(_ui->dimensionSpinBox->value() - 1);   // Dimension to display
        Metric::Chart* chart = new Metric::Chart( _metric->id(), d, MS::selectedModel());
        chart->addTitle();
        _cview->setChart( chart);
    }   // end if
    else
    {
        static const QString src0 = "Growth curve statistics N/A";
        QString src = src0;
        QtCharts::QChart* chart = new QtCharts::QChart;
        QString mname = _metric ? _metric->name() : "No current measurement";
        chart->setTitle( QString("<center><big><b>%1</b><br><em>%2</em></big></center>").arg( mname, src));
        _cview->setChart(chart);
    }   // end else

    _ui->saveButton->setEnabled( hasMC);
}   // end _doRefreshChart


namespace {
bool hasValidExtension( const QString& fname)
{
    return fname.endsWith( ".jpg", Qt::CaseInsensitive)
        || fname.endsWith( ".jpeg", Qt::CaseInsensitive)
        || fname.endsWith( ".png", Qt::CaseInsensitive)
        || fname.endsWith( ".bmp", Qt::CaseInsensitive)
        || fname.endsWith( ".svg", Qt::CaseInsensitive);
}   // end hasValidExtension
}   // end namespace


void ChartDialog::_doOnSaveImage()
{
    _fdialog->selectFile( "chart.png");
    QString imgpath;
    if ( _fdialog->exec())
        imgpath = _fdialog->selectedFiles().first().trimmed();

    if ( !imgpath.isEmpty() && !saveImage( _cview, imgpath))
    {
        const QString msg = QString("Unable to save to '%1'").arg(imgpath);
        QMB::warning( this, tr("Save Error!"), QString("<p align='center'>%1</p>").arg(msg));
    }   // end if
}   // end _doOnSaveImage


namespace {
void paintChartToDevice( QtCharts::QChartView *cview, QPaintDevice *pdev)
{
    QPainter painter;
    painter.begin( pdev);
    painter.setRenderHint( QPainter::Antialiasing);
    cview->render( &painter);
}   // end paintChartToDevice
}   // end namespace


bool ChartDialog::saveImage( QtCharts::QChartView *cview, QString &fpath)
{
    assert( !fpath.isEmpty());

    bool savedOkay = false;

    QString suffix = QFileInfo(fpath).suffix().toLower();
    if ( !hasValidExtension(fpath))
    {
        suffix = "jpg";
        fpath += "." + suffix;
    }   // end if

    QSvgGenerator simg;
    const QSize sz = cview->size();
    simg.setSize( sz);
    simg.setViewBox( QRect( 0, 0, sz.width(), sz.height()));
    simg.setResolution( 90);

    if ( suffix == "svg")
    {
        QSaveFile saveFile( fpath);
        if ( saveFile.open(QIODevice::WriteOnly))
        {
            simg.setOutputDevice( &saveFile);
            paintChartToDevice( cview, &simg);
            savedOkay = saveFile.commit();
        }   // end if
    }   // end if
    else
    {
        QBuffer buffer;
        if ( buffer.open(QIODevice::WriteOnly))
        {
            // First write the SVG out to internal buffer
            simg.setOutputDevice( &buffer);
            paintChartToDevice( cview, &simg);
            buffer.close();

            // Then write the buffer out to image data via SVG renderer
            QSvgRenderer svgRenderer( buffer.data());
            QPixmap img( sz);
            img.fill( suffix == "png" ? Qt::transparent : Qt::white);
            QPainter painter;
            painter.begin( &img);
            painter.setRenderHint( QPainter::Antialiasing);
            svgRenderer.render( &painter);
            savedOkay = img.save( fpath);
        }   // end if
    }   // end else

    return savedOkay;
}   // end saveImage

