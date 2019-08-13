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
#include <PhenotypeManager.h>
#include <FaceModelManager.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <TreeModel.h>
#include <QImageTools.h>
#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>
#include <cmath>
#include <cassert>
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::Metric::MetricCalculatorManager;
using FaceTools::Metric::PhenotypeManager;
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
    _dialogRootTitle( parent->windowTitle() + " | Assessment Information")
{
    _ui->setupUi(this);

    setWindowTitle( _dialogRootTitle);
    //connect( _ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScanInfoDialog::_doOnApply);

    connect( _ui->maternalEthnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnChangedMaternalEthnicity);
    connect( _ui->paternalEthnicityComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnChangedPaternalEthnicity);
    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnSexChanged);

    connect( _ui->dobDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnDOBChanged);
    connect( _ui->captureDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnCaptureDateChanged);

    connect( _ui->sourceLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnSourceChanged);
    connect( _ui->studyIdLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnStudyIdChanged);

    connect( _ui->notesTextEdit, &QPlainTextEdit::textChanged, this, &ScanInfoDialog::_doOnNotesChanged);
    connect( _ui->assessorComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnAssessorChanged);
    connect( _ui->assessorComboBox, &QComboBox::editTextChanged, this, &ScanInfoDialog::_doOnEditedAssessorText);

    connect( _ui->addAssessmentButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnAddAssessment);
    connect( _ui->removeAssessmentButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnDeleteAssessment);
    connect( _ui->copyLandmarksButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnCopyLandmarks);

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
    _ui->imageLabel->clear();
    _ui->assessorComboBox->clear();

    _aids.clear();

    if (_model)
    {
        const QString fpath = FileIO::FMM::filepath(_model).c_str();
        setWindowTitle( _dialogRootTitle + " | " + fpath);

        _ui->dobDateEdit->setDate( _model->dateOfBirth());
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( _model->sex()));

        _setEthnicityComboBox( _ui->maternalEthnicityComboBox, _model->maternalEthnicity());
        _setEthnicityComboBox( _ui->paternalEthnicityComboBox, _model->paternalEthnicity());

        _ui->captureDateEdit->setDate( _model->captureDate());
        _ui->sourceLineEdit->setText( _model->source());
        _ui->studyIdLineEdit->setText( _model->studyId());

        const IntSet aids = _model->assessmentIds();
        for ( int ai : aids)
        {
            _aids.insert(ai);
            FaceAssessment::CPtr ass = _model->assessment(ai);
            const QString aname = ass->assessor();
            _ui->assessorComboBox->addItem( aname, ai);
        }   // end for
    }   // end if

    _refreshCurrentAssessment();

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
    if ( _model->source() != src)
        _model->setSource( src);

    const QString sid = _ui->studyIdLineEdit->text();
    if ( _model->studyId() != sid)
        _model->setStudyId( sid);

    const int aid = _ui->assessorComboBox->currentData().toInt();

    const QString ass = _ui->assessorComboBox->currentText();
    if ( _model->assessment(aid)->assessor() != ass)    // Change name of assessor?
    {
        _model->assessment(aid)->setAssessor( ass);
        _model->setMetaSaved(false);
    }   // end if

    const QString rem = _ui->notesTextEdit->toPlainText();
    if ( _model->assessment(aid)->notes() != rem)   // Change assessment notes?
    {
        _model->assessment(aid)->setNotes( rem);
        _model->setMetaSaved(false);
    }   // end if

    const bool isSaved = _model->isSaved();
    _model->unlock();

    if ( !isSaved)
        emit onUpdated(_model);
}   // end _doOnApply


bool ScanInfoDialog::_isDifferentToCurrent()
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
    if ( _model->source() != src)
        return true;

    const QString sid = _ui->studyIdLineEdit->text();
    if ( _model->studyId() != sid)
        return true;

    const int aid = _ui->assessorComboBox->currentData().toInt();

    const QString ass = _ui->assessorComboBox->currentText();
    if ( _model->assessment(aid)->assessor() != ass)
        return true;

    const QString rem = _ui->notesTextEdit->toPlainText();
    if ( _model->assessment(aid)->notes() != rem)
        return true;

    return false;
}   // end _isDifferentToCurrent


void ScanInfoDialog::_checkEnableApply()
{
    const bool isdiff = _isDifferentToCurrent();
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(isdiff);
}   // end _checkEnableApply


void ScanInfoDialog::_doOnChangedMaternalEthnicity()
{
    assert(_model != nullptr);
    // If user landed on the "Aggregated" option, set back to the current ethnicity.
    if (_ui->maternalEthnicityComboBox->currentText() == "Aggregated")
        _setEthnicityComboBox( _ui->maternalEthnicityComboBox, _model->maternalEthnicity());
    _checkEnableApply();
}   // end _doOnChangedMaternalEthnicity


void ScanInfoDialog::_doOnChangedPaternalEthnicity()
{
    assert(_model != nullptr);
    // If user landed on the "Aggregated" option, set back to the current ethnicity.
    if (_ui->paternalEthnicityComboBox->currentText() == "Aggregated")
        _setEthnicityComboBox( _ui->paternalEthnicityComboBox, _model->paternalEthnicity());
    _checkEnableApply();
}   // end _doOnChangedPaternalEthnicity


void ScanInfoDialog::_doOnDOBChanged()
{
    if ( _model && _ui->dobDateEdit->date() > QDate::currentDate())   // Can't have dates beyond current date!
        _ui->dobDateEdit->setDate( _model->dateOfBirth());
    _checkEnableApply();
}   // end _doOnDOBChanged


void ScanInfoDialog::_doOnCaptureDateChanged()
{
    if ( _model && _ui->captureDateEdit->date() > QDate::currentDate())   // Can't have dates beyond current date!
        _ui->captureDateEdit->setDate( _model->captureDate());
    _checkEnableApply();
}   // end _doOnCaptureDateChanged


void ScanInfoDialog::_doOnSexChanged()
{
    _checkEnableApply();
}   // end _doOnSexChanged


void ScanInfoDialog::_doOnSourceChanged()
{
    _checkEnableApply();
}   // end _doOnSourceChanged


void ScanInfoDialog::_doOnStudyIdChanged()
{
    _checkEnableApply();
}   // end _doOnStudyIdChanged


void ScanInfoDialog::_doOnNotesChanged()
{
    _checkEnableApply();
}   // end _doOnNotesChanged


void ScanInfoDialog::_doOnAssessorChanged()
{
    const int ai = _ui->assessorComboBox->currentData().toInt();
    if ( _model && ai >= 0)
    {
        _model->setCurrentAssessment(ai);
        _refreshCurrentAssessment();
        emit onAssessmentChanged();
    }   // end if
}   // end _doOnAssessorChanged


void ScanInfoDialog::_doOnEditedAssessorText()
{
    //const int ai = _ui->assessorComboBox->currentData().toInt();
    _checkEnableApply();
}   // end _doOnEditedAssessorText


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


void ScanInfoDialog::_refreshCurrentAssessment()
{
    _ui->notesTextEdit->clear();
    _ui->addAssessmentButton->setEnabled(false);
    _ui->removeAssessmentButton->setEnabled(false);
    _ui->copyLandmarksButton->setEnabled(false);

    QStringList pterms;
    if ( _model)
    {
        FaceAssessment::CPtr cass = _model->currentAssessment();
        assert(cass);
        _ui->assessorComboBox->setCurrentIndex( _ui->assessorComboBox->findData( cass->id()));
        _ui->notesTextEdit->setPlainText( cass->notes());
        _ui->addAssessmentButton->setEnabled( true);
        _ui->removeAssessmentButton->setEnabled( _model->assessmentsCount() > 1);
        _ui->copyLandmarksButton->setEnabled( !cass->landmarks().empty() && _model->assessmentsCount() > 1);

        const IntSet ptypes = PhenotypeManager::discover( _model, cass->id());
        for ( int hid : ptypes)
            pterms << PhenotypeManager::phenotype(hid)->name();
    }   // end if

    QString msg = tr("No atypical phenotypic variations identified.");
    if ( !pterms.isEmpty())
        msg = pterms.join("; ");
    _ui->hpoTermsLabel->setText(msg);
}   // end refreshCurrentAssessment


void ScanInfoDialog::_doOnAddAssessment()
{
    assert( _model);

    // Set the new assessment ID to be one higher than all existing
    int naid = 0;
    for ( int aid : _aids)
        naid = std::max(naid,aid);
    naid++;

    // Create the new assessment, add to record of assessment IDs and insert in combobox at current index
    FaceAssessment::Ptr nass = FaceAssessment::create( naid, "Unknown");
    _aids.insert( nass->id());
    const int idx = _ui->assessorComboBox->currentIndex();
    _ui->assessorComboBox->insertItem( idx, nass->assessor(), nass->id());

    // If the current assessment has landmarks and/or paths, copy these in to the new assessment.
    FaceAssessment::CPtr cass = _model->currentAssessment();
    if ( cass->hasLandmarks())
        nass->setLandmarks( cass->landmarks().deepCopy());
    if ( cass->hasPaths())
        nass->setPaths( cass->paths().deepCopy());

    // Change model and refresh
    _model->setAssessment(nass);
    _model->setCurrentAssessment(nass->id());
    _model->setMetaSaved(false);
    _refreshCurrentAssessment();
    // Set focus to the assessor's name which the user will want to change
    _ui->assessorComboBox->setFocus();
    emit onUpdated(_model);
    emit onAssessmentChanged();
}   // end _doOnAddAssessment


void ScanInfoDialog::_doOnDeleteAssessment()
{
    const int ai = _ui->assessorComboBox->currentData().toInt();
    assert( ai >= 0);
    assert( _model);
    assert( _model->assessmentsCount() > 1);

    FaceAssessment::CPtr ass = _model->assessment(ai);

    bool dodel = true;
    // Warn about deletion if the assessment has notes, paths, or landmarks defined.
    if ( ass->hasLandmarks() || ass->hasPaths() || ass->hasNotes())
    {
        static const QString msg = tr("This action will erase this assessment's landmarks, custom paths, and notes! Really delete?");
        dodel = QMessageBox::Yes == QMessageBox::warning( this, tr("Delete Assessment?"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if

    if ( dodel)
    {
        // Remove from the combo box
        _ui->assessorComboBox->removeItem(_ui->assessorComboBox->currentIndex());   // Changes current index
        _aids.erase(ai);

        // Change model and refresh
        _model->eraseAssessment(ai);
        _model->setCurrentAssessment( _ui->assessorComboBox->currentData().toInt());
        _model->setMetaSaved(false);
        _refreshCurrentAssessment();
        emit onUpdated(_model);
        emit onAssessmentChanged();
    }   // end if
}   // end _doOnDeleteAssessment


void ScanInfoDialog::_doOnCopyLandmarks()
{
    const int ai = _ui->assessorComboBox->currentData().toInt();
    FaceAssessment::CPtr sass = _model->assessment(ai);
    assert( ai >= 0);
    assert( _model);
    assert( _model->assessmentsCount() > 1);
    assert( sass->hasLandmarks());

    // Warn about copying if to do so will overwrite other assessment landmark sets.
    bool dowarn = false;
    for ( int aid : _aids)
    {
        if ( ai != aid && _model->assessment(aid)->hasLandmarks())
        {
            dowarn = true;
            break;
        }   // end if
    }   // end for

    bool docopy = true;
    if ( dowarn)
    {
        static const QString msg = tr("This action will overwrite existing landmarks for other assessments! Really overwrite?");
        docopy = QMessageBox::Yes == QMessageBox::warning( this, tr("Overwrite Landmarks?"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    }   // end if

    if ( docopy)
    {
        for ( int aid : _aids)
        {
            if ( ai != aid)
            {
                FaceAssessment::Ptr lass = _model->assessment(aid);
                lass->setLandmarks( sass->landmarks().deepCopy());
            }   // end if
        }   // end for

        _model->setMetaSaved(false);
        emit onCopiedLandmarks();
        emit onUpdated(_model);
    }   // end if
}   // end _doOnCopyLandmarks
