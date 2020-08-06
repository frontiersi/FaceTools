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

#ifndef FACE_TOOLS_ACTION_ACTION_UPDATE_GROWTH_DATA_H
#define FACE_TOOLS_ACTION_ACTION_UPDATE_GROWTH_DATA_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionUpdateGrowthData : public FaceAction
{ Q_OBJECT
public:
    ActionUpdateGrowthData();

    static bool setStatsToModel( const FM*);
    static bool setAutoStats( bool v);
    static bool autoStats() { return s_autoStats;}

protected:
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    static bool s_autoStats;
};  // end class

}}   // end namespaces

#endif
