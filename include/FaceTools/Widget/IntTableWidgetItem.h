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

#ifndef FACETOOLS_WIDGET_INT_TABLE_WIDGET_ITEM_H
#define FACETOOLS_WIDGET_INT_TABLE_WIDGET_ITEM_H

// For columns of properly sorted int data.

#include <FaceTools/FaceTypes.h>
#include <QTableWidgetItem>

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT IntTableWidgetItem : public QTableWidgetItem
{
public:
    explicit IntTableWidgetItem( int v, int rightJustifyFieldWidth=0);

    bool operator<( const QTableWidgetItem& other) const override;
};  // end class

}}   // end namespace

#endif
