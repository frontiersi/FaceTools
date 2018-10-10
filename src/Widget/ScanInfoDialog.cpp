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

#include <ScanInfoDialog.h>
#include <ui_ScanInfoDialog.h>
#include <FaceModel.h>
#include <QImageTools.h>
#include <QPushButton>
#include <cmath>
#include <cassert>
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::FM;
using FaceTools::Sex;


ScanInfoDialog::ScanInfoDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::ScanInfoDialog), _model(nullptr)
{
    ui->setupUi(this);
    connect( ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScanInfoDialog::doOnApply);
    set(nullptr);
}   // end ctor


ScanInfoDialog::~ScanInfoDialog()
{
    delete ui;
}   // end dtor


void ScanInfoDialog::set( FM* fm)
{
    if ( fm == _model && fm && fm->isSaved())
        return;

    setAge( 0);
    setSex( UNKNOWN_SEX);
    setEthnicity("");
    setProvenance("");
    setRemarks("");
    ui->numVerticesLabel->setText("");
    ui->numPolygonsLabel->setText("");

    _model = fm;
    if (fm)
    {
        setAge( fm->age());
        setSex( fm->sex());
        setEthnicity( fm->ethnicity());
        setCaptureDate( fm->captureDate());
        setProvenance( fm->source());
        setRemarks( fm->description());
        const RFeatures::ObjModel* cmodel = fm->info()->cmodel();
        ui->numVerticesLabel->setText(QString("%1 vertices  ").arg(cmodel->getNumVertices()));
        ui->numPolygonsLabel->setText(QString("%1 polygons  ").arg(cmodel->getNumFaces()));
    }   // end if

    setThumbnail();
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(fm != nullptr);
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(fm != nullptr);
}   // end set


double ScanInfoDialog::age() const
{
    double yrs = ui->yearsSpinBox->value();
    double mths = ui->monthsSpinBox->value();
    return yrs + mths/12;
}   // end age

std::string ScanInfoDialog::ethnicity() const { return ui->ethnicityLineEdit->text().toStdString();}
Sex ScanInfoDialog::sex() const { return static_cast<Sex>(ui->sexComboBox->currentIndex());}
QDate ScanInfoDialog::captureDate() const { return ui->captureDateEdit->date();}

std::string ScanInfoDialog::source() const { return ui->sourceLineEdit->text().toStdString();}
std::string ScanInfoDialog::description() const { return ui->remarksTextEdit->toPlainText().toStdString();}


// private
void ScanInfoDialog::setAge( double a)
{
    const double yrs = floor(a);
    ui->yearsSpinBox->setValue(static_cast<int>(yrs));
    const double mths = floor( 12.0 * (a - yrs));
    ui->monthsSpinBox->setValue(static_cast<int>(mths));
}   // end setAge


void ScanInfoDialog::setSex( Sex s) { ui->sexComboBox->setCurrentIndex( static_cast<int16_t>(s));}
void ScanInfoDialog::setEthnicity( const std::string& e) { ui->ethnicityLineEdit->setText(e.c_str());}
void ScanInfoDialog::setCaptureDate( const QDate& d) { ui->captureDateEdit->setDate(d);}
void ScanInfoDialog::setProvenance( const std::string& t) { ui->sourceLineEdit->setText( t.c_str());}
void ScanInfoDialog::setRemarks( const std::string& t) { ui->remarksTextEdit->setPlainText( t.c_str());}


void ScanInfoDialog::setThumbnail()
{
    const size_t I = static_cast<size_t>( std::min<int>( ui->imageLabel->size().height(), ui->imageLabel->size().width()));
    QImage thumbnail;
    if ( _model)
        thumbnail = QTools::copyOpenCV2QImage( _model->thumbnail(I));
    ui->imageLabel->setPixmap( QPixmap::fromImage(thumbnail));
}   // end setThumbnail


void ScanInfoDialog::doOnApply()
{
    assert(_model != nullptr);

    _model->setAge(age());
    _model->setEthnicity(ethnicity());
    _model->setSex(sex());
    _model->setCaptureDate(captureDate());
    _model->setSource(source());
    _model->setDescription(description());

    if ( !_model->isSaved())
        emit onUpdated(_model);
}   // end doOnApply


void ScanInfoDialog::accept()
{
    doOnApply();
    this->hide();
}   // end accept


void ScanInfoDialog::reject()
{
    set(_model);
    this->hide();
}   // end reject
