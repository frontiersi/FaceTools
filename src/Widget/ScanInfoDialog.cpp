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
#include <FaceModelManager.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <TreeModel.h>
#include <QImageTools.h>
#include <QTreeWidget>
#include <QPushButton>
#include <cmath>
#include <cassert>
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::Metric::MetricCalculatorManager;
using FaceTools::FM;

/*
// Never allow automatic insertion!
class EthnicityValidator : public QValidator
{ public:
    EthnicityValidator( QObject* o) : QValidator(o) {}
    QValidator::State validate( QString&, int&) const override { return QValidator::Invalid;}
};  // end EthnicityValidator
*/

namespace {

void _setEthnicityComboBox( QComboBox* cb, int ecode=0)
{
    static const IntSet EMPTY_INT_SET;

    std::list<int> path;  // Will be path from root to leaf (ecode)
    path.push_front(ecode);

    // Progressively work backwards to the broadest category setting the discovered path:
    const IntSet* pcodes = &FaceTools::Ethnicities::parentCodes(ecode);
    while ( !pcodes->empty())
    {
        // Find the non-mixed parent.
        int nmp = 0;
        for ( int pcode : *pcodes)
        {
            // There's only 1 non-mixed parent per ethnic code
            if ( !FaceTools::Ethnicities::isMixed(pcode))
            {
                nmp = pcode;
                break;
            }   // end if
        }   // end for

        pcodes = &EMPTY_INT_SET;
        if ( nmp > 0)
        {
            path.push_front( nmp);
            // Will return empty when the code at the front is of the "broad" category.
            pcodes = &FaceTools::Ethnicities::parentCodes( nmp);
        }   // end if
    }   // end while

    QModelIndex midx = QModelIndex();
    cb->setRootModelIndex( midx);   // Initialise to root
    int rowIdx = 0;

    if ( ecode != 0)
    {
        if ( FaceTools::Ethnicities::isMixed(ecode))
        {
            midx = cb->model()->index( 1, 0, midx); // Set to the aggregate option
            cb->setRootModelIndex(midx);
        }   // end else

        for ( int ec : path)
        {
            const QString& ename = FaceTools::Ethnicities::name(ec);
            rowIdx = cb->findData( ename, Qt::DisplayRole);
            //std::cerr << QString("Found '%1' at row index %2").arg(ename).arg(rowIdx).toStdString() << std::endl;
            if ( ec != ecode)
            {
                midx = cb->model()->index( rowIdx, 0, midx);
                cb->setRootModelIndex(midx);
            }   // end if
        }   // end for
    }   // end if

    cb->setCurrentIndex( rowIdx);
}   // end _setEthnicityComboBox


QTools::TreeModel* createEthnicityComboBoxModel()
{
    QTools::TreeModel *emodel = new QTools::TreeModel;
    QTools::TreeItem *root = emodel->setNewRoot({"Cultural and Ethnic Group"});
    root->appendChild( new QTools::TreeItem( {"Not stated"}));
    QTools::TreeItem *supplm = new QTools::TreeItem( {"Aggregated"}, root);
    QTools::TreeItem *cbroad = nullptr; // Current broad node
    QTools::TreeItem *cnrrow = nullptr; // Current narrow node

    for ( int ethn : FaceTools::Ethnicities::codes())
    {
        if ( ethn <= 0)
            continue;

        const QString ename = FaceTools::Ethnicities::name(ethn);

        if ( FaceTools::Ethnicities::isMixed( ethn))
            supplm->appendChild( new QTools::TreeItem( {ename}));
        else if ( FaceTools::Ethnicities::isBroad(ethn))    // 1000, 2000, 3000 etc
            cbroad = new QTools::TreeItem( {ename}, root);
        else if ( FaceTools::Ethnicities::isNarrow(ethn))  // 1100, 1200, 1300 etc
        {
            assert( cbroad != nullptr);
            cnrrow = new QTools::TreeItem( {ename}, cbroad);
        }   // end else if
        else
        {
            assert( cnrrow != nullptr);
            cnrrow->appendChild( new QTools::TreeItem( {ename}));
        }   // end else
    }   // end for

    return emodel;
}   // end createEthnicityComboBoxModel

}   // end namespace


ScanInfoDialog::ScanInfoDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ScanInfoDialog), _model(nullptr),
    _dialogRootTitle( parent->windowTitle() + " | Subject Information")
{
    _ui->setupUi(this);
    setWindowTitle( _dialogRootTitle);
    connect( _ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScanInfoDialog::_doOnApply);

    connect( _ui->maternalEthnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnChangedMaternalEthnicity);
    connect( _ui->paternalEthnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnChangedPaternalEthnicity);
    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnSexChanged);

    connect( _ui->dobDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnDOBChanged);
    connect( _ui->captureDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnCaptureDateChanged);

    connect( _ui->sourceLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnSourceChanged);
    connect( _ui->studyIdLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnStudyIdChanged);
    connect( _ui->remarksTextEdit, &QPlainTextEdit::textChanged, this, &ScanInfoDialog::_doOnNotesChanged);

    QTools::TreeModel* emodel = createEthnicityComboBoxModel();
    _ui->maternalEthnicityComboBox->setModel( emodel);
    _ui->paternalEthnicityComboBox->setModel( emodel);

    _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX | MALE_SEX), FEMALE_SEX | MALE_SEX);    // Used for intersex
    _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX), FEMALE_SEX);
    _ui->sexComboBox->addItem( toLongSexString( MALE_SEX), MALE_SEX);

    refresh();
}   // end ctor


ScanInfoDialog::~ScanInfoDialog()
{
    delete _ui;
}   // end dtor


void ScanInfoDialog::set( FM* fm)
{
    _model = fm;
    refresh();
}   // end set


void ScanInfoDialog::refresh()
{
    setWindowTitle( _dialogRootTitle);
    _ui->dobDateEdit->setDate( QDate::currentDate());
    _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( FEMALE_SEX | MALE_SEX));

    _setEthnicityComboBox( _ui->maternalEthnicityComboBox);
    _setEthnicityComboBox( _ui->paternalEthnicityComboBox);

    _ui->captureDateEdit->setDate( QDate::currentDate());
    _ui->sourceLineEdit->clear();
    _ui->studyIdLineEdit->clear();
    _ui->remarksTextEdit->clear();
    _ui->imageLabel->clear();

    if (_model)
    {
        _model->lockForRead();
        const QString fpath = FileIO::FMM::filepath(_model).c_str();
        setWindowTitle( _dialogRootTitle + " | " + fpath);

        _ui->dobDateEdit->setDate( _model->dateOfBirth());
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( _model->sex()));

        _setEthnicityComboBox( _ui->maternalEthnicityComboBox, _model->maternalEthnicity());
        _setEthnicityComboBox( _ui->paternalEthnicityComboBox, _model->paternalEthnicity());

        _ui->captureDateEdit->setDate( _model->captureDate());
        _ui->sourceLineEdit->setText( _model->source());
        _ui->studyIdLineEdit->setText( _model->studyId());
        _ui->remarksTextEdit->setPlainText( _model->notes());
        _model->unlock();
    }   // end if

    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(_model != nullptr);
}   // end refresh


void ScanInfoDialog::setThumbnail( const cv::Mat_<cv::Vec3b> img)
{
    QImage qimg = QTools::copyOpenCV2QImage( img);
    _ui->imageLabel->setPixmap( QPixmap::fromImage(qimg));
}   // end setThumbnail


QSize ScanInfoDialog::thumbDims() const
{
    return _ui->imageLabel->size();
}   // end thumbDims


void ScanInfoDialog::_doOnApply()
{
    assert(_model != nullptr);

    _model->lockForWrite();
    if ( _ui->dobDateEdit->date() <= QDate::currentDate())
        _model->setDateOfBirth( _ui->dobDateEdit->date());

    const int methn = Ethnicities::code( _ui->maternalEthnicityComboBox->currentText());
    if ( methn != _model->maternalEthnicity())
        _model->setMaternalEthnicity(methn);

    const int pethn = Ethnicities::code( _ui->paternalEthnicityComboBox->currentText());
    if ( pethn != _model->paternalEthnicity())
        _model->setPaternalEthnicity(pethn);

    const int8_t sex = static_cast<int8_t>(_ui->sexComboBox->currentData().toInt());
    if ( sex != _model->sex())
        _model->setSex( sex);

    const QDate date = _ui->captureDateEdit->date();
    if ( date != _model->captureDate())
        _model->setCaptureDate(date);

    const QString src = _ui->sourceLineEdit->text();
    const QString sid = _ui->studyIdLineEdit->text();
    const QString rem = _ui->remarksTextEdit->toPlainText();

    if ( _model->source() != src)
        _model->setSource( src);

    if ( _model->studyId() != sid)
        _model->setStudyId( sid);

    if ( _model->notes() != rem)
        _model->setNotes(rem);

    const bool isSaved = _model->isSaved();
    _model->unlock();

    if ( !isSaved)
        emit onUpdated(_model);
}   // end _doOnApply


bool ScanInfoDialog::_isDifferentToModel()
{
    if ( !_model)
        return false;

    if ( _ui->dobDateEdit->date() != _model->dateOfBirth())
        return true;

    const int methn = Ethnicities::code( _ui->maternalEthnicityComboBox->currentText());
    if ( methn != _model->maternalEthnicity())
        return true;

    const int pethn = Ethnicities::code( _ui->paternalEthnicityComboBox->currentText());
    if ( pethn != _model->paternalEthnicity())
        return true;

    const int8_t sex = static_cast<int8_t>(_ui->sexComboBox->currentData().toInt());
    if ( sex != _model->sex())
        return true;

    const QDate date = _ui->captureDateEdit->date();
    if ( date != _model->captureDate())
        return true;

    const QString src = _ui->sourceLineEdit->text();
    const QString sid = _ui->studyIdLineEdit->text();
    const QString rem = _ui->remarksTextEdit->toPlainText();

    if ( _model->source() != src)
        return true;

    if ( _model->studyId() != sid)
        return true;

    if ( _model->notes() != rem)
        return true;

    return false;
}   // end _isDifferentToModel


void ScanInfoDialog::_doOnChangedMaternalEthnicity()
{
    assert(_model != nullptr);
    // If user landed on the "Aggregated" option, set back to the current ethnicity.
    if (_ui->maternalEthnicityComboBox->currentText() == "Aggregated")
        _setEthnicityComboBox( _ui->maternalEthnicityComboBox, _model->maternalEthnicity());
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnChangedMaternalEthnicity


void ScanInfoDialog::_doOnChangedPaternalEthnicity()
{
    assert(_model != nullptr);
    // If user landed on the "Aggregated" option, set back to the current ethnicity.
    if (_ui->paternalEthnicityComboBox->currentText() == "Aggregated")
        _setEthnicityComboBox( _ui->paternalEthnicityComboBox, _model->paternalEthnicity());
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnChangedPaternalEthnicity


void ScanInfoDialog::_doOnDOBChanged()
{
    if ( _model && _ui->dobDateEdit->date() > QDate::currentDate())   // Can't have dates beyond current date!
        _ui->dobDateEdit->setDate( _model->dateOfBirth());
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnDOBChanged


void ScanInfoDialog::_doOnCaptureDateChanged()
{
    if ( _model && _ui->captureDateEdit->date() > QDate::currentDate())   // Can't have dates beyond current date!
        _ui->captureDateEdit->setDate( _model->captureDate());
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnCaptureDateChanged


void ScanInfoDialog::_doOnSexChanged()
{
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnSexChanged


void ScanInfoDialog::_doOnSourceChanged()
{
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnSourceChanged


void ScanInfoDialog::_doOnStudyIdChanged()
{
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnStudyIdChanged


void ScanInfoDialog::_doOnNotesChanged()
{
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(_isDifferentToModel());
}   // end _doOnNotesChanged



void ScanInfoDialog::accept()
{
    _doOnApply();
    QDialog::accept();
}   // end accept


void ScanInfoDialog::reject()
{
    refresh();
    QDialog::reject();
}   // end reject
