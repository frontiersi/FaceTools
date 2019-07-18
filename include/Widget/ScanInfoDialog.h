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

#ifndef FACETOOLS_WIDGET_SCAN_INFO_DIALOG_H
#define FACETOOLS_WIDGET_SCAN_INFO_DIALOG_H

#include <FaceTypes.h>
#include <QDialog>
#include <QValidator>

namespace Ui { class ScanInfoDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT ScanInfoDialog : public QDialog
{ Q_OBJECT
public:
    explicit ScanInfoDialog( QWidget *parent=nullptr);
    ~ScanInfoDialog() override;

    void set( FM*);
    const FM* get() const { return _model;}

    QSize thumbDims() const;
    void setThumbnail( const cv::Mat_<cv::Vec3b>);

signals:
    void onUpdated( FM*);   // On updated data on FaceModel

public slots:
    void accept() override;
    void reject() override;
    void refresh();

private slots:
    void _doOnApply();
    void _doOnChangedMaternalEthnicity();
    void _doOnChangedPaternalEthnicity();
    void _doOnDOBChanged();
    void _doOnCaptureDateChanged();
    void _doOnSexChanged();
    void _doOnSourceChanged();
    void _doOnStudyIdChanged();
    void _doOnNotesChanged();

private:
    Ui::ScanInfoDialog *_ui;
    FM *_model;
    const QString _dialogRootTitle;
    bool _isDifferentToModel();
};  // end class

}}   // end namespace

#endif
