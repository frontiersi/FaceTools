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

#include <CheckAllTableHeader.h>
using FaceTools::Widget::CheckAllTableHeader;

CheckAllTableHeader::CheckAllTableHeader( QWidget *parent, bool useEye)
    : QHeaderView( Qt::Horizontal, parent), _on(false), _useEye(useEye)
{
    setSectionsClickable(true);
    connect( this, &QHeaderView::sectionClicked, this, [this](int lidx){ if (lidx == 0){ setAllChecked(!_on);}});
}   // end ctor


void CheckAllTableHeader::setAllChecked( bool v)
{
    if ( _on != v)
    {
        _on = v;
        this->showSection(0);
        emit allChecked(_on);
    }   // end if
}   // end setAllChecked


void CheckAllTableHeader::paintSection( QPainter *painter, const QRect &rect, int lidx) const
{
    painter->save();
    QHeaderView::paintSection( painter, rect, lidx);
    painter->restore();
    if (lidx == 0)
    {
        QStyleOptionButton option;
        const double ct = double(rect.height())/6;
        const int hh = int( 4*ct + 0.5);

        option.rect = QRect( int(ct), int(ct), hh, hh);
        option.state = _on ? QStyle::State_On : QStyle::State_Off;

        if ( _useEye)
        {
            QString visIcon = hh <= 20 ? ":/icons/VISIBLE_SMALL" : ":/icons/VISIBLE";
            QString invisIcon = hh <= 20 ? ":/icons/INVISIBLE_SMALL" : ":/icons/INVISIBLE";
            QPixmap pmap = _on ? QPixmap( visIcon).scaled(hh,hh) : QPixmap(invisIcon).scaled(hh,hh);
            style()->drawItemPixmap( painter, option.rect, 0, pmap);
        }   // end if
        else
            style()->drawPrimitive( QStyle::PE_IndicatorCheckBox, &option, painter);
    }   // end if
}   // end paintSection
