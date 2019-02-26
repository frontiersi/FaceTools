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
#include <MetricCalculatorManager.h>
#include <FaceModel.h>
#include <QImageTools.h>
#include <QPushButton>
#include <cmath>
#include <cassert>
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::Metric::MetricCalculatorManager;
using FaceTools::FM;
using FaceTools::Sex;


// Never allow automatic insertion!
class EthnicityValidator : public QValidator
{ public:
    EthnicityValidator( QObject* o) : QValidator(o) {}
    QValidator::State validate( QString&, int&) const override { return QValidator::Invalid;}
};  // end EthnicityValidator



ScanInfoDialog::ScanInfoDialog(QWidget *parent) :
    QDialog(parent), _ui(new Ui::ScanInfoDialog), _model(nullptr)
{
    _ui->setupUi(this);
    connect( _ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScanInfoDialog::doOnApply);
    set(nullptr);

    _ui->ethnicityComboBox->addItem("N/A");
    _ethnicities.insert( "n/a");
    for ( const QString& ethn : MetricCalculatorManager::ethnicities())
        addEthnicityToComboBox( ethn);
    _ui->ethnicityComboBox->setCurrentIndex(0);
    _ui->ethnicityComboBox->setEditable(true);
    _ui->ethnicityComboBox->setValidator( new EthnicityValidator(_ui->ethnicityComboBox));

    _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX | MALE_SEX));    // Used for intersex
    _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX));
    _ui->sexComboBox->addItem( toLongSexString( MALE_SEX));
    _ui->sexComboBox->setCurrentIndex(0);
}   // end ctor


ScanInfoDialog::~ScanInfoDialog()
{
    delete _ui;
}   // end dtor


void ScanInfoDialog::set( FM* fm)
{
    if ( fm == _model && fm && fm->isSaved())
        return;

    _model = fm;
    reset();
}   // end set


// private
QString ScanInfoDialog::addEthnicityToComboBox( QString eth)
{
    if ( eth.isEmpty())
        eth = "N/A";
    QString lethn = eth.toLower();
    if ( _ethnicities.count( lethn) == 0)
    {
         _ethnicities.insert(lethn);

         // Find where in the combo box to insert
         const int n = _ui->ethnicityComboBox->count();
         int i = 1;
         for ( i = 1; i < n; ++i)
         {
            QString e = _ui->ethnicityComboBox->itemText(i);
            if ( e >= eth)
                break;
         }  // end for
        _ui->ethnicityComboBox->insertItem( i, eth);
    }   // end if
    return eth;
}   // end addEthnicityToComboBox


// private
void ScanInfoDialog::reset()
{
    _ui->dobDateEdit->setDate( QDate::currentDate());
    _ui->sexComboBox->setCurrentText( toLongSexString( FEMALE_SEX | MALE_SEX));
    _ui->ethnicityComboBox->setCurrentText("N/A");
    _ui->captureDateEdit->setDate( QDate::currentDate());
    _ui->sourceLineEdit->clear();
    _ui->studyIdLineEdit->clear();
    _ui->remarksTextEdit->clear();
    _ui->numVerticesLabel->clear();
    _ui->numPolygonsLabel->clear();
    _ui->imageLabel->clear();

    if (_model)
    {
        _ui->dobDateEdit->setDate( _model->dateOfBirth());
        _ui->sexComboBox->setCurrentText( toLongSexString( _model->sex()));
        _ui->ethnicityComboBox->setCurrentText( addEthnicityToComboBox( _model->ethnicity()));
        _ui->captureDateEdit->setDate( _model->captureDate());
        _ui->sourceLineEdit->setText( _model->source());
        _ui->studyIdLineEdit->setText( _model->studyId());
        _ui->remarksTextEdit->setPlainText( _model->notes());
        const RFeatures::ObjModel* cmodel = _model->info()->cmodel();
        _ui->numVerticesLabel->setText(QString("%1 vertices  ").arg(cmodel->getNumVertices()));
        _ui->numPolygonsLabel->setText(QString("%1 polygons  ").arg(cmodel->getNumFaces()));
    }   // end if

    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_model != nullptr);
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(_model != nullptr);
}   // end reset


// public
void ScanInfoDialog::setThumbnail( const cv::Mat_<cv::Vec3b>& img)
{
    QImage qimg;
    if ( _model)
    {
        //cv::Mat_<cv::Vec3b> img = FaceTools::makeThumbnail( _model, I);
        qimg = QTools::copyOpenCV2QImage( img);
    }   // end if
    _ui->imageLabel->setPixmap( QPixmap::fromImage(qimg));
}   // end setThumbnail


// public
int ScanInfoDialog::minThumbDims() const
{
    return std::min<int>( _ui->imageLabel->size().height(), _ui->imageLabel->size().width());
}   // end minThumbDims


void ScanInfoDialog::doOnApply()
{
    assert(_model != nullptr);

    if ( _ui->dobDateEdit->date() <= QDate::currentDate())
    {
        _model->setDateOfBirth( _ui->dobDateEdit->date());
        _model->setSaved(false);
    }   // end if

    QString ethn = _ui->ethnicityComboBox->currentText();
    if ( ethn != "N/A")
        addEthnicityToComboBox(ethn);
    else if ( ethn == "N/A")
        ethn = "";

    if ( ethn != _model->ethnicity())
    {
        _model->setEthnicity(ethn);
        _model->setSaved(false);
    }   // end if

    const int8_t sex = fromLongSexString(_ui->sexComboBox->currentText());
    if ( sex != _model->sex())
    {
        _model->setSex( sex);
        _model->setSaved(false);
    }   // end if

    const QDate date = _ui->captureDateEdit->date();
    if ( date != _model->captureDate())
    {
        _model->setCaptureDate(date);
        _model->setSaved(false);
    }   // end if

    const QString src = _ui->sourceLineEdit->text();
    const QString sid = _ui->studyIdLineEdit->text();
    const QString rem = _ui->remarksTextEdit->toPlainText();

    if ( _model->source() != src)
    {
        _model->setSource( src);
        _model->setSaved( false);
    }   // end if

    if ( _model->studyId() != sid)
    {
        _model->setStudyId( sid);
        _model->setSaved(false);
    }   // end if

    if ( _model->notes() != rem)
    {
        _model->setNotes(rem);
        _model->setSaved(false);
    }   // end if

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
    reset();
    this->hide();
}   // end reject
