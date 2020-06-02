/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ************************************************************************/

#ifndef FACE_TOOLS_ACTION_CHANGE_SURFACE_MAPPING_RANGE_H
#define FACE_TOOLS_ACTION_CHANGE_SURFACE_MAPPING_RANGE_H

#include "FaceAction.h"
#include <QTools/RangeMinMax.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionChangeSurfaceMappingRange : public FaceAction
{ Q_OBJECT
public:
    explicit ActionChangeSurfaceMappingRange( const QString&);

    QWidget* widget() const override { return _widget;}

    QString toolTip() const override { return "Change the range over which the selected surface visualisation is applied.";}

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool checkState( Event) override;

private slots:
    void _doOnWidgetValueChanged( float, float);

private:
    QTools::RangeMinMax *_widget;
};  // end class

}}   // end namespaces

#endif
