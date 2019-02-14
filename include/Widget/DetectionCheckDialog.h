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

#ifndef FACETOOLS_WIDGET_DETECTION_CHECK_DIALOG_H
#define FACETOOLS_WIDGET_DETECTION_CHECK_DIALOG_H

#include <FaceTypes.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class DetectionCheckDialog;}

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT DetectionCheckDialog : public QDialog
{ Q_OBJECT
public:
    explicit DetectionCheckDialog(QWidget *parent = nullptr);
    ~DetectionCheckDialog() override;

    // Ids of last accepted landmarks checked off.
    const IntSet& landmarks() const { return _landmarks;}

    bool open( const FM*);

private slots:
    void doOnSetAllChecked( bool);
    void doOnItemChanged( QTableWidgetItem*);
    void sortOnColumn( int);

private:
    Ui::DetectionCheckDialog *_ui;
    std::unordered_map<int, int> _idRows;  // ID --> Row index
    IntSet _landmarks;  // Ids of landmarks to update

    void populate();
    void appendRow(int);
    void resetIdRowMap();
};  // end class

}}  // end namespaces

#endif
