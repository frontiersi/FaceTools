/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_TOGGLE_LEGEND_H
#define FACE_TOOLS_ACTION_TOGGLE_LEGEND_H

#include "FaceAction.h"
#include <r3dvis/ScalarLegend.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionToggleLegend : public FaceAction
{ Q_OBJECT
public:
    explicit ActionToggleLegend( const QString&);

    QString toolTip() const override { return "Toggle the surface colour mapping legend off/on.";}

protected:
    void postInit() override;
    bool isAllowed( Event) override;
    bool update( Event) override;

private:
    std::unordered_map<const FMV*, r3dvis::ScalarLegend> _legends;
};  // end class

}}   // end namespaces

#endif
