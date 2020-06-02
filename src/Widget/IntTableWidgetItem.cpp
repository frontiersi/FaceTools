/************************************************************************
 * Copyright (C) 2018 SIS Research Ltd & Richard Palmer
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

#include <Widget/IntTableWidgetItem.h>
using FaceTools::Widget::IntTableWidgetItem;

IntTableWidgetItem::IntTableWidgetItem( int v, int rjfw)
    : QTableWidgetItem( rjfw > 0 ? QString("%1").arg(v).rightJustified(rjfw) : QString("%1").arg(v)) {}

bool IntTableWidgetItem::operator<( const QTableWidgetItem& other) const
{
    return text().toInt() < other.text().toInt();
}   // end operator<
