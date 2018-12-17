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

#ifndef FACETOOLS_WIDGET_PHENOTYPES_DIALOG_H
#define FACETOOLS_WIDGET_PHENOTYPES_DIALOG_H

#include <FaceTypes.h>
#include <QTableWidgetItem>
#include <QDialog>

namespace Ui { class PhenotypesDialog; }

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT PhenotypesDialog : public QDialog
{ Q_OBJECT
public:
    explicit PhenotypesDialog( QWidget *parent = nullptr);
    ~PhenotypesDialog() override;

public slots:
    void doOnShowPhenotypes( const IntSet&);

private slots:
    void doOnUserSelectedSyndrome();
    void sortOnColumn(int);

private:
    Ui::PhenotypesDialog *_ui;
    int _chid;
    std::unordered_map<int, int> _idRows;  // ID --> Row index
    IntSet _dhids;

    void appendRow( int);
    void populateSyndromes( const IntSet&);
    void highlightRow(int);
    void setSelectedPhenotype(int);
};  // end class

}}  // end namespaces

#endif
