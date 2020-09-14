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

#ifndef FACE_TOOLS_ACTION_TOGGLE_AXES_H
#define FACE_TOOLS_ACTION_TOGGLE_AXES_H

#include "FaceAction.h"
#include <FaceTools/FaceModelViewer.h>
#include <vtkCubeAxesActor.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionToggleAxes : public FaceAction
{ Q_OBJECT
public:
    ActionToggleAxes( const QString&, const QIcon&, const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Toggle the display of the axes grid.";}

protected:
    void postInit() override;
    bool update( Event) override;
    bool isAllowed( Event) override;

private:
    std::unordered_map<FMV*, vtkNew<vtkCubeAxesActor> > _viewers;
    void _addViewer( FMV*);
};  // end class

}}   // end namespaces

#endif
