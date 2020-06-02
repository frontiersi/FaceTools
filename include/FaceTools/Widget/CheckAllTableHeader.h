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

#ifndef FACETOOLS_WIDGET_CHECK_ALL_TABLE_HEADER_H
#define FACETOOLS_WIDGET_CHECK_ALL_TABLE_HEADER_H

#include <FaceTools/FaceTypes.h>
#include <QtGui>
#include <QHeaderView>

namespace FaceTools { namespace Widget {

class FaceTools_EXPORT CheckAllTableHeader : public QHeaderView
{ Q_OBJECT
public:
    CheckAllTableHeader( QWidget *parent = nullptr, bool useEye=false);

public slots:
    void setAllChecked( bool);

signals:
    void allChecked(bool);

protected:
    void paintSection( QPainter*, const QRect&, int) const;

private:
    bool _on;
    bool _useEye;
};  // end class

}}   // end namespaces

#endif
