/************************************************************************
 * Copyright (C) 2022 SIS Research Ltd & Richard Palmer
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

#include <Widget/ScanInfoDialog.h>
#include <ui_ScanInfoDialog.h>
#include <Metric/PhenotypeManager.h>
#include <FileIO/FaceModelDatabase.h>
#include <FileIO/FaceModelManager.h>
#include <ModelSelect.h>
#include <Ethnicities.h>
#include <FaceModel.h>
#include <FaceTools.h>
#include <QTreeWidget>
#include <QPushButton>
#include <QSignalBlocker>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <cmath>
#include <cassert>
using FaceTools::Widget::ScanInfoDialog;
using FaceTools::Ethnicities;
using FaceTools::FM;
using FMM = FaceTools::FileIO::FaceModelManager;
using FMD = FaceTools::FileIO::FaceModelDatabase;
using MS = FaceTools::ModelSelect;
using QMB = QMessageBox;

/*
// Never allow automatic insertion!
class EthnicityValidator : public QValidator
{ public:
    EthnicityValidator( QObject* o) : QValidator(o) {}
    QValidator::State validate( QString&, int&) const override { return QValidator::Invalid;}
};  // end EthnicityValidator
*/

ScanInfoDialog::ScanInfoDialog( QWidget *parent) :
    QDialog(parent), _ui(new Ui::ScanInfoDialog),
    _dialogRootTitle( parent->windowTitle() + " | Assessment Information"),
    _focusOutSignaller( QEvent::FocusOut, false)
{
    _ui->setupUi(this);

    setWindowTitle( _dialogRootTitle);
    connect( _ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &ScanInfoDialog::_apply);

    // Use currentIndexChanged because of selection mechanics of the tree model.
    connect( _ui->maternalEthnicityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                       this, &ScanInfoDialog::_doOnChangedMaternalEthnicity);
    connect( _ui->paternalEthnicityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                                       this, &ScanInfoDialog::_doOnChangedPaternalEthnicity);
    connect( _ui->sexComboBox, QOverload<int>::of(&QComboBox::activated), this, &ScanInfoDialog::_doOnSexChanged);

    _ui->dobDateEdit->setDisplayFormat( "dd MMM yyyy");
    _ui->captureDateEdit->setDisplayFormat( "dd MMM yyyy");
    connect( _ui->dobDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnDOBChanged);
    connect( _ui->captureDateEdit, &QDateEdit::dateChanged, this, &ScanInfoDialog::_doOnCaptureDateChanged);

    connect( _ui->sourceLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnSourceChanged);
    connect( _ui->studyIdLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnStudyIdChanged);
    connect( _ui->imageIdLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnImageIdChanged);
    connect( _ui->subjectIdLineEdit, &QLineEdit::textEdited, this, &ScanInfoDialog::_doOnSubjectIdChanged);

    connect( _ui->notesTextEdit, &QPlainTextEdit::textChanged, this, &ScanInfoDialog::_doOnNotesChanged);

    connect( _ui->assessorComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
                            this, &ScanInfoDialog::_doOnAssessorChanged);
    _focusOutSignaller.install( _ui->assessorComboBox);
    connect( &_focusOutSignaller, &QTools::EventSignaller::onEvent, [this](){ _checkEnableApply();});

    _ui->assessorComboBox->setInsertPolicy( QComboBox::NoInsert);

    // One or more words separated by single spaces
    const QRegularExpression re( "\\b[^\\d\\W]+(\\b\\s?[^\\d\\W]*)*");
    QRegularExpressionValidator *nameValidator = new QRegularExpressionValidator( re);
    _ui->assessorComboBox->setValidator( nameValidator);

    connect( _ui->addAssessmentButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnCopyAssessment);
    connect( _ui->removeAssessmentButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnDeleteAssessment);
    connect( _ui->copyLandmarksButton, &QToolButton::clicked, this, &ScanInfoDialog::_doOnCopyLandmarks);

    QTools::TreeModel* emodel = Ethnicities::createComboBoxModel();
    _ui->maternalEthnicityComboBox->setModel( emodel);
    _ui->paternalEthnicityComboBox->setModel( emodel);

    _ui->sexComboBox->addItem( toLongSexString( UNKNOWN_SEX), UNKNOWN_SEX);
    _ui->sexComboBox->addItem( toLongSexString( FEMALE_SEX), FEMALE_SEX);
    _ui->sexComboBox->addItem( toLongSexString( MALE_SEX), MALE_SEX);

    this->adjustSize();
    setFixedSize( geometry().width(), geometry().height());

    refresh();
}   // end ctor



ScanInfoDialog::~ScanInfoDialog()
{
    delete _ui;
}   // end dtor


QString ScanInfoDialog::_resetSubjectIdentifier()
{
    const FM *fm = MS::selectedModel();
    const QString sid = fm ? fm->subjectId() : "";
    QSignalBlocker blocker( _ui->subjectIdLineEdit);
    _ui->subjectIdLineEdit->setText( sid);
    return sid;
}   // end _resetSubjectIdentifier


QString ScanInfoDialog::_resetSubjectData()
{
    const QString sid = _resetSubjectIdentifier();
    QSignalBlocker b1( _ui->dobDateEdit);
    QSignalBlocker b2( _ui->sexComboBox);
    QSignalBlocker b3( _ui->maternalEthnicityComboBox);
    QSignalBlocker b4( _ui->paternalEthnicityComboBox);

    const FM *fm = MS::selectedModel();
    if (fm)
    {
        _ui->dobDateEdit->setDate( fm->dateOfBirth());
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( fm->sex()));
        Ethnicities::resetComboBox( _ui->maternalEthnicityComboBox, fm->maternalEthnicity());
        Ethnicities::resetComboBox( _ui->paternalEthnicityComboBox, fm->paternalEthnicity());
    }   // end if
    else
    {
        _ui->dobDateEdit->setDate( QDate::currentDate());
        _ui->sexComboBox->setCurrentIndex( _ui->sexComboBox->findData( UNKNOWN_SEX));
        Ethnicities::resetComboBox( _ui->maternalEthnicityComboBox);
        Ethnicities::resetComboBox( _ui->paternalEthnicityComboBox);
    }   // end else

    return sid;
}   // end _resetSubjectData


void ScanInfoDialog::_resetImageData()
{
    QSignalBlocker b0( _ui->captureDateEdit);
    QSignalBlocker b1( _ui->sourceLineEdit);
    QSignalBlocker b2( _ui->studyIdLineEdit);
    QSignalBlocker b3( _ui->imageIdLineEdit);
    QSignalBlocker b4( _ui->assessorComboBox);

    const FM *fm = MS::selectedModel();
    if (fm)
    {
        _ui->captureDateEdit->setDate( fm->captureDate());
        _ui->sourceLineEdit->setText( fm->source());
        _ui->studyIdLineEdit->setText( fm->studyId());
        _ui->imageIdLineEdit->setText( fm->imageId());
        for ( int ai : fm->assessmentIds())
        {
            FaceAssessment::CPtr ass = fm->assessment(ai);
            _ui->assessorComboBox->addItem( ass->assessor(), ai);
        }   // end for
    }   // end if
    else
    {
        _ui->captureDateEdit->setDate( QDate::currentDate());
        _ui->sourceLineEdit->clear();
        _ui->studyIdLineEdit->clear();
        _ui->imageIdLineEdit->clear();
        _ui->assessorComboBox->clear();
    }   // end else
}   // end _resetImageData


void ScanInfoDialog::refresh()
{
    QString winTitle = _dialogRootTitle;
    const FM *fm = MS::selectedModel();
    if (fm)
        winTitle = _dialogRootTitle + " | " + FMM::filepath(*fm);
    setWindowTitle( winTitle);
    _resetSubjectData();
    _resetImageData();
    _ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled( false);
    _ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( fm != nullptr);
    refreshAssessment();
}   // end refresh


void ScanInfoDialog::setThumbnail( const QPixmap &pmap) { _ui->imageLabel->setPixmap( pmap);}

QSize ScanInfoDialog::thumbDims() const { return _ui->imageLabel->size();}


void ScanInfoDialog::_apply()
{
    FM *fm = MS::selectedModel();
    assert( fm);

    bool subjectDataChanged = false;
    fm->lockForWrite();

    /**** SUBJECT DETAILS ****/
    QDate dob = _ui->dobDateEdit->date();
    if ( dob != fm->dateOfBirth())
        subjectDataChanged = true;

    int8_t sex = static_cast<int8_t>(_ui->sexComboBox->currentData().toInt());
    if ( sex != fm->sex())
        subjectDataChanged = true;

    int meth = Ethnicities::code( _ui->maternalEthnicityComboBox->currentText());
    if ( meth != fm->maternalEthnicity())
        subjectDataChanged = true;

    int peth = Ethnicities::code( _ui->paternalEthnicityComboBox->currentText());
    if ( peth != fm->paternalEthnicity())
        subjectDataChanged = true;

    QString nsid = _ui->subjectIdLineEdit->text().trimmed();    // Get the subject ID (may not be saved on image)
    if ( nsid.isEmpty())    // New empty identifiers not allowed!
        nsid = _resetSubjectIdentifier();

    const bool isUnknownSubject = FMD::NO_SUBJECT_REGEXP.exactMatch(nsid);
    // If an invalid subject identifier is set, reset the subject data.
    if ( isUnknownSubject)
    {
        if ( subjectDataChanged)
        {
            static const QString msg = tr("A valid identifier must be set to change the subject's details!");
            QMB::warning( this, tr("Invalid Subject Identifer!"), QString("<p align='center'>%1</p>").arg(msg));
            subjectDataChanged = false;
        }   // end if
        nsid = _resetSubjectData();
    }   // end if
    else if ( nsid != fm->subjectId())
    {
        // If the new subject ID matches an existing subject, ask the user if they want to use the same data
        // as already exists in the database for the subject, otherwise reset the subject identifier.
        int8_t nsex;
        QDate ndob;
        int nmeth, npeth;
        if ( FMD::subjectMeta( nsid, nsex, ndob, nmeth, npeth) && ( nsex != sex || ndob != dob || nmeth != meth || npeth != peth))
        {
            static const QString msg = tr("<p>This subject already exists with different details!</p><p>Use the existing subject's details instead (YES)?<br>Or set a different subject identifier (NO)?</p>");
            if ( QMB::information( this, tr("Use Existing Subject Details?"), msg, QMB::Yes | QMB::No, QMB::Yes) == QMB::Yes)
            {
                sex = nsex;
                dob = ndob;
                meth = nmeth;
                peth = npeth;
            }   // end if
            else
            {
                QSignalBlocker b0( _ui->subjectIdLineEdit);
                _ui->subjectIdLineEdit->setText( fm->subjectId());
                _ui->subjectIdLineEdit->setFocus();
                nsid = fm->subjectId();
            }   // end else
        }   // end if
    }   // end else if

    if ( nsid != fm->subjectId())
        subjectDataChanged = true;

    // Check for mismatching subject details
    if ( subjectDataChanged)
    {
        bool confirmUpdate = true;
        // If at least one other image has the same subject ID but different metadata.
        // Warn that applying this change will update all other images with the same subject in the database.
        const size_t numImgs = FMD::numImages(nsid);    // Num images using this subject identifier
        if ( (numImgs > 1 && nsid == fm->subjectId()) && !FMD::isSubjectMetaMatched( nsid, sex, dob, meth, peth))
        {
            static const QString msg = tr("<p>Multiple images with the same subject identifier have mismatching subject details! Applying these changes will update <em>all images of this subject</em> in the database to use these details. To fix the changes you must load then save each affected 3DF file.</p><p align='center'>Apply the changes to all %1 images?</p>").arg(numImgs);
            confirmUpdate = QMB::warning( this, tr("Update Subject Everywhere?!"),
                                          QString("<p>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No) == QMB::Yes;
        }   // end if

        if ( confirmUpdate)
        {
            fm->setSubjectId( nsid);
            fm->setSex( sex);
            fm->setPaternalEthnicity(peth);
            fm->setMaternalEthnicity(meth);
            fm->setDateOfBirth( dob);

            // If the other loaded image has the same subject identifier, make all the same changes
            FM *ofm = MS::nonSelectedModel();
            if ( ofm && ofm->subjectId() == nsid)
            {
                ofm->setSubjectId( nsid);
                ofm->setSex( sex);
                ofm->setPaternalEthnicity(peth);
                ofm->setMaternalEthnicity(meth);
                ofm->setDateOfBirth( dob);
                ofm->setMetaSaved(false);
            }   // end if
        }   // end if
        else
        {
            _resetSubjectData();
            subjectDataChanged = false;
        }   // end else
    }   // end if
    /*************************/

    bool dataChanged = subjectDataChanged;

    /**** IMAGE DETAILS ****/
    const QDate date = _ui->captureDateEdit->date();
    if ( date != fm->captureDate())
        dataChanged = true;
    const QString src = _ui->sourceLineEdit->text();
    if ( fm->source() != src)
        dataChanged = true;
    const QString sid = _ui->studyIdLineEdit->text();
    if ( fm->studyId() != sid)
        dataChanged = true;
    const QString iid = _ui->imageIdLineEdit->text();
    if ( fm->imageId() != iid)
        dataChanged = true;
    /*************************/

    /**** ASSESSMENT DETAILS ****/
    const int aid = _ui->assessorComboBox->currentData().toInt();
    const QString ass = _ui->assessorComboBox->currentText().trimmed();
    if ( fm->assessment(aid)->assessor() != ass)    // Change name of assessor?
    {
        _ui->assessorComboBox->setItemText( _ui->assessorComboBox->findData(aid), ass);
        dataChanged = true;
    }   // end if
    const QString rem = _ui->notesTextEdit->toPlainText();
    if ( fm->assessment(aid)->notes() != rem)   // Change assessment notes?
        dataChanged = true;
    /****************************/

    if ( dataChanged)
    {
        fm->setCaptureDate(date);
        fm->setSource( src);
        fm->setStudyId( sid);
        fm->setImageId( iid);
        fm->assessment( aid)->setAssessor( ass);
        fm->assessment( aid)->setNotes( rem);
        FMD::refreshImage( *fm, "", "", subjectDataChanged);
        fm->setMetaSaved(false);
    }   // end if

    const bool isSaved = fm->isSaved();
    fm->unlock();

    if ( !isSaved)
    {
        emit onInfoChanged();
        refresh();
    }   // end if

    _checkEnableApply();
}   // end _apply


bool ScanInfoDialog::_isDifferentToCurrent() const
{
    FM::RPtr fm = MS::selectedModelScopedRead();
    if ( !fm)
        return false;

    if ( _ui->dobDateEdit->date() != fm->dateOfBirth())
        return true;

    if ( fm->maternalEthnicity() != Ethnicities::code( _ui->maternalEthnicityComboBox->currentText()))
        return true;

    if ( fm->paternalEthnicity() != Ethnicities::code( _ui->paternalEthnicityComboBox->currentText()))
        return true;

    if ( fm->sex() != static_cast<int8_t>(_ui->sexComboBox->currentData().toInt()))
        return true;

    if ( fm->captureDate() != _ui->captureDateEdit->date())
        return true;

    if ( fm->source() != _ui->sourceLineEdit->text())
        return true;

    if ( fm->studyId() != _ui->studyIdLineEdit->text())
        return true;

    if ( fm->subjectId() != _ui->subjectIdLineEdit->text())
        return true;

    if ( fm->imageId() != _ui->imageIdLineEdit->text())
        return true;

    const int aid = _ui->assessorComboBox->currentData().toInt();

    if ( fm->assessment(aid)->assessor() != _ui->assessorComboBox->currentText().trimmed())
        return true;

    if ( fm->assessment(aid)->notes() != _ui->notesTextEdit->toPlainText())
        return true;

    return false;
}   // end _isDifferentToCurrent


void ScanInfoDialog::_checkEnableApply()
{
    const bool diffToCurrent = _isDifferentToCurrent();
    _ui->buttonBox->button( QDialogButtonBox::Apply)->setEnabled( diffToCurrent);
}   // end _checkEnableApply


bool ScanInfoDialog::isApplyEnabled() const
{
    return _ui->buttonBox->button( QDialogButtonBox::Apply)->isEnabled();
}   // end isApplyEnabled


void ScanInfoDialog::_doOnDOBChanged()
{
    const QDate capDate = _ui->captureDateEdit->date();
    if ( MS::isViewSelected() && _ui->dobDateEdit->date() > capDate)   // Can't have dates beyond image capture date!
    {
        QSignalBlocker b0( _ui->dobDateEdit);
        _ui->dobDateEdit->setDate( capDate);
    }   // end if
    _checkEnableApply();
}   // end _doOnDOBChanged


void ScanInfoDialog::_doOnCaptureDateChanged()
{
    // Can't have capture date later than current date or before date of birth.
    if ( MS::isViewSelected())
    {
        QSignalBlocker b0( _ui->dobDateEdit);
        QSignalBlocker b1( _ui->captureDateEdit);
        const QDate capDate = _ui->captureDateEdit->date();
        const QDate dobDate = _ui->dobDateEdit->date();
        if ( capDate > QDate::currentDate())
            _ui->captureDateEdit->setDate( QDate::currentDate());
        else if ( capDate < dobDate)
            _ui->dobDateEdit->setDate( capDate);
    }   // end if
    _checkEnableApply();
}   // end _doOnCaptureDateChanged


void ScanInfoDialog::_doOnChangedMaternalEthnicity() { _checkEnableApply();}
void ScanInfoDialog::_doOnChangedPaternalEthnicity() { _checkEnableApply();}
void ScanInfoDialog::_doOnSexChanged() { _checkEnableApply();}
void ScanInfoDialog::_doOnSourceChanged() { _checkEnableApply();}
void ScanInfoDialog::_doOnStudyIdChanged() { _checkEnableApply();}
void ScanInfoDialog::_doOnSubjectIdChanged() { _checkEnableApply();}
void ScanInfoDialog::_doOnImageIdChanged() { _checkEnableApply();}
void ScanInfoDialog::_doOnNotesChanged() { _checkEnableApply();}
//void ScanInfoDialog::_doOnEditedAssessorText() { std::cerr << "Edited" << std::endl; _checkEnableApply();}


void ScanInfoDialog::_doOnAssessorChanged()
{
    FM *fm = MS::selectedModel();
    const int ai = _ui->assessorComboBox->currentData().toInt();
    if ( fm && ai != fm->currentAssessment()->id())
    {
        fm->lockForWrite();
        fm->setCurrentAssessment(ai);
        fm->unlock();
        refreshAssessment();
        emit onInfoChanged();
    }   // end if
}   // end _doOnAssessorChanged


void ScanInfoDialog::accept()
{
    if ( _isDifferentToCurrent())
        _apply();
    QDialog::accept();
}   // end accept


void ScanInfoDialog::reject()
{
    refresh();
    QDialog::reject();
}   // end reject


void ScanInfoDialog::refreshAssessment()
{
    QSignalBlocker b0( _ui->notesTextEdit);
    QSignalBlocker b1( _ui->assessorComboBox);

    _ui->notesTextEdit->clear();
    _ui->addAssessmentButton->setEnabled(false);
    _ui->removeAssessmentButton->setEnabled(false);
    _ui->copyLandmarksButton->setEnabled(false);

    FM::RPtr fm = MS::selectedModelScopedRead();
    if ( fm)
    {
        FaceAssessment::CPtr cass = fm->currentAssessment();
        _ui->assessorComboBox->setCurrentIndex( _ui->assessorComboBox->findData( cass->id()));
        _ui->notesTextEdit->setPlainText( cass->notes());
        _ui->notesTextEdit->moveCursor( QTextCursor::Start);
        _ui->addAssessmentButton->setEnabled( true);
        _ui->removeAssessmentButton->setEnabled( fm->assessmentsCount() > 1);
        _ui->copyLandmarksButton->setEnabled( !cass->landmarks().empty() && fm->assessmentsCount() > 1);
        refreshNotableHPOs();
    }   // end if
}   // end refreshAssessment


void ScanInfoDialog::refreshNotableHPOs()
{
    const FM *fm = MS::selectedModel();
    QString msg;
    if ( !fm)
        msg = tr("<center><b>No model loaded!</b></center>");
    else if ( !fm->hasLandmarks())
        msg = tr("<center><b>Detect face for phenotypic assessment</b></center>");
    else
    {
        const IntSet hids = Metric::PhenotypeManager::discover( *fm, fm->currentAssessment()->id());
        QStringList lterms;
        for ( int hid : hids)
            lterms << Metric::PhenotypeManager::htmlLinkString(hid);
        lterms.sort();  // Sort into alphanumeric order

        if ( !lterms.isEmpty())
            msg = tr("<b>Noted phenotypic traits: </b><ul type=\"disc\"><li>") + lterms.join("<li>") + "</ul>";
        else
            msg = tr("<b>No noted phenotypic traits found.</b>");
    }   // end else
    _ui->hpoTermsTextBrowser->document()->setHtml(msg);
    _ui->hpoTermsTextBrowser->moveCursor( QTextCursor::Start);
}   // end refreshNotableHPOs


void ScanInfoDialog::_doOnCopyAssessment()
{
    FM *fm = MS::selectedModel();
    assert(fm);
    fm->lockForRead();

    // Get the new assessment ID to be one higher than all existing
    int naid = 0;
    for ( int aid : fm->assessmentIds())
        naid = std::max(naid,aid);
    naid++;

    FaceAssessment::CPtr cass = fm->currentAssessment();

    // Copy assessment
    FaceAssessment::Ptr nass = cass->deepCopy();
    nass->setId( naid);
    // Set the name to something unique
    nass->setAssessor("");
    nass->setAssessor( QString( "%1_%2").arg( nass->assessor()).arg(naid));
    // Insert in combobox at current index
    const std::string prevAssessorName = _ui->assessorComboBox->currentText().toStdString();
    const int idx = _ui->assessorComboBox->currentIndex();
    _ui->assessorComboBox->insertItem( idx, nass->assessor(), nass->id());

    // If the new assessment has landmarks and/or paths, record this for the notes field.
    std::string notes;
    if ( nass->hasLandmarks())
        notes = "Landmarks";
    if ( nass->hasPaths())
    {
        if ( !notes.empty())
            notes += " and c";
        else
            notes += "C";
        notes += "alliper measurements";
    }   // end if

    // Set the notes on the new assessment to say that these data were copied over.
    if ( !notes.empty())
        notes += " copied over from assessment " + prevAssessorName + ".";
    nass->setNotes( tr(notes.c_str()));

    fm->unlock();
    fm->lockForWrite();
    // Change model and refresh
    fm->setAssessment(nass);
    fm->setCurrentAssessment(nass->id());
    fm->setMetaSaved(false);
    fm->unlock();
    refreshAssessment();
    // Set focus to the assessor's name which the user will want to change
    _ui->assessorComboBox->setFocus();
    emit onInfoChanged();
}   // end _doOnCopyAssessment


void ScanInfoDialog::_doOnDeleteAssessment()
{
    FM *fm = MS::selectedModel();
    assert(fm);
    fm->lockForRead();

    const int ai = _ui->assessorComboBox->currentData().toInt();
    assert( ai >= 0);
    assert( fm->assessmentsCount() > 1);

    FaceAssessment::CPtr ass = fm->assessment(ai);

    bool dodel = true;
    // Warn about deletion if the assessment has notes, paths, or landmarks defined.
    if ( ass->hasLandmarks() || ass->hasPaths() || ass->hasNotes())
    {
        static const QString msg = tr("Really erase this assessment's landmarks, calliper measurements, and notes? This can't be undone!");
        dodel = QMB::warning( this, tr("Delete Assessment?"),
                QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No) == QMB::Yes;
    }   // end if
    fm->unlock();

    if ( dodel)
    {
        fm->lockForWrite();
        // Remove from the combo box
        QSignalBlocker blk(_ui->assessorComboBox);
        _ui->assessorComboBox->removeItem(_ui->assessorComboBox->currentIndex());   // Changes current index
        // Change model and refresh
        fm->eraseAssessment(ai);
        fm->setCurrentAssessment( _ui->assessorComboBox->currentData().toInt());
        fm->setMetaSaved(false);
        fm->unlock();
        refreshAssessment();
        emit onInfoChanged();
    }   // end if
}   // end _doOnDeleteAssessment


void ScanInfoDialog::_doOnCopyLandmarks()
{
    FM *fm = MS::selectedModel();
    assert(fm);
    fm->lockForRead();
    const int ai = _ui->assessorComboBox->currentData().toInt();
    FaceAssessment::CPtr sass = fm->assessment(ai);
    assert( ai >= 0);
    assert( fm->assessmentsCount() > 1);
    assert( sass->hasLandmarks());

    // Warn about copying if to do so will overwrite other assessment landmark sets.
    bool dowarn = false;
    for ( int aid : fm->assessmentIds())
    {
        if ( ai != aid && fm->assessment(aid)->hasLandmarks())
        {
            dowarn = true;
            break;
        }   // end if
    }   // end for

    bool docopy = true;
    if ( dowarn)
    {
        static const QString msg = tr("Are you sure you want to overwrite the landmarks of other assessments? This action cannot be undone!");
        docopy = QMB::warning( this, tr("Overwrite Landmarks?"),
                      QString("<p align='center'>%1</p>").arg(msg), QMB::Yes | QMB::No, QMB::No) == QMB::Yes;
    }   // end if

    fm->unlock();

    if ( docopy)
    {
        fm->lockForWrite();
        for ( int aid : fm->assessmentIds())
        {
            if ( ai != aid)
            {
                FaceAssessment::Ptr lass = fm->assessment(aid);
                lass->setLandmarks( sass->landmarks());
            }   // end if
        }   // end for

        fm->setMetaSaved(false);
        fm->unlock();
        emit onInfoChanged();
    }   // end if
}   // end _doOnCopyLandmarks
