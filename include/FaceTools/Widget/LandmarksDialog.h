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

#ifndef FACETOOLS_WIDGET_LANDMARKS_DIALOG_H
#define FACETOOLS_WIDGET_LANDMARKS_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <FaceTools/Action/FaceAction.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class LandmarksDialog; }

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT LandmarksDialog : public QDialog
{ Q_OBJECT
public:
    explicit LandmarksDialog( QWidget *parent = nullptr);
    ~LandmarksDialog() override;

    void setShowAction( Action::FaceAction*);
    void setAlignAction( Action::FaceAction*);
    void setLabelsAction( Action::FaceAction*);
    void setRestoreAction( Action::FaceAction*);

    void setMessage( const QString&);
    QString message() const;

signals:
    void onShowLandmarkLabels( bool);
    void onAlignLandmarks();
    void onRestoreLandmarks();

public slots:
    void show();
    void setSelectedLandmark( int);

protected:
    void showEvent( QShowEvent*) override;
    void closeEvent( QCloseEvent*) override;

private slots:
    void _doOnSetAllChecked( bool);
    void _doOnItemChanged( QTableWidgetItem*);
    void _doOnSetTableRow( int);
    void _doSortOnColumn( int);
    void _doOnRefreshTable();

private:
    Ui::LandmarksDialog *_ui;
    std::unordered_map<int, int> _idRows;
    int _prowid;

    void _populate();
    void _appendRow( int);
    void _highlightRow( int);
    void _triggerShow( bool);
};  // end class

}}  // end namespaces

#endif
