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

#ifndef FACE_TOOLS_ACTION_ACTION_UPDATE_MEASUREMENTS_H
#define FACE_TOOLS_ACTION_ACTION_UPDATE_MEASUREMENTS_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionUpdateMeasurements : public FaceAction
{ Q_OBJECT
public:
    ActionUpdateMeasurements();

    static bool updateAllMeasurements( FM*);

    static bool updateMeasurementsForLandmark( FM*, int lmid);

    static bool updateMeasurementsForLandmarks( FM*, const IntSet&);

protected:
    void postInit() override;
    void doAction( Event) override;
    Event doAfterAction( Event) override { return _ev;}

private:
    Event _ev;
};  // end class

}}   // end namespaces

#endif
