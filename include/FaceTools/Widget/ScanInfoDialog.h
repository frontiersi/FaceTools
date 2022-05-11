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

#ifndef FACETOOLS_WIDGET_SCAN_INFO_DIALOG_H
#define FACETOOLS_WIDGET_SCAN_INFO_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <QTools/EventSignaller.h>
#include <QDialog>
#include <opencv2/opencv.hpp>

namespace Ui { class ScanInfoDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ScanInfoDialog : public QDialog
{ Q_OBJECT
public:
    explicit ScanInfoDialog( QWidget *parent=nullptr);
    ~ScanInfoDialog() override;

    QSize thumbDims() const;
    void setThumbnail( const QPixmap&);

    bool isApplyEnabled() const;

    void refreshNotableHPOs();

signals:
    void onInfoChanged();

public slots:
    void accept() override;
    void reject() override;
    void refresh(); // Refresh all info
    void refreshAssessment(); // Just refresh assessment info

private slots:
    void _doOnChangedMaternalEthnicity();
    void _doOnChangedPaternalEthnicity();
    void _doOnDOBChanged();
    void _doOnCaptureDateChanged();
    void _doOnSexChanged();
    void _doOnSourceChanged();
    void _doOnStudyIdChanged();
    void _doOnImageIdChanged();
    void _doOnSubjectIdChanged();
    void _doOnNotesChanged();
    void _doOnAssessorChanged();

    void _doOnCopyAssessment();
    void _doOnDeleteAssessment();
    void _doOnCopyLandmarks();

    void _apply();

private:
    Ui::ScanInfoDialog *_ui;
    const QString _dialogRootTitle;
    QTools::EventSignaller _focusOutSignaller;
    bool _isDifferentToCurrent() const;
    void _checkEnableApply();
    void _resetSubjectData();
    void _resetImageData();
};  // end class

}}   // end namespace

#endif
