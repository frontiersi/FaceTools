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

#ifndef FACETOOLS_WIDGET_LANDMARKS_CHECK_DIALOG_H
#define FACETOOLS_WIDGET_LANDMARKS_CHECK_DIALOG_H

#include <FaceTools/FaceTypes.h>
#include <FaceTools/FaceModel.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class LandmarksCheckDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT LandmarksCheckDialog : public QDialog
{ Q_OBJECT
public:
    explicit LandmarksCheckDialog(QWidget *parent = nullptr);
    ~LandmarksCheckDialog() override;

    // Ids of last accepted landmarks checked off.
    const IntSet& landmarks() const { return _landmarks;}

    bool open( const FM&);

private slots:
    void _doOnSetAllChecked( bool);
    void _doOnItemChanged( QTableWidgetItem*);
    void _doSortOnColumn( int);

private:
    Ui::LandmarksCheckDialog *_ui;
    std::unordered_map<int, int> _idRows;  // ID --> Row index
    IntSet _landmarks;  // Ids of landmarks to update

    void _populate();
    void _appendRow(int);
};  // end class

}}  // end namespaces

#endif
