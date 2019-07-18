/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_EDIT_PATHS_H
#define FACE_TOOLS_ACTION_EDIT_PATHS_H

#include "ActionVisualise.h"
#include <PathSetVisualisation.h>
#include <PathsInteractor.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionEditPaths : public ActionVisualise
{ Q_OBJECT
public:
    ActionEditPaths( const QString&, const QIcon&,
                     Vis::PathSetVisualisation*,
                     Interactor::PathsInteractor::Ptr,
                     const QKeySequence& ks=QKeySequence());

    QString toolTip() const override { return "Toggle the showing of any custom path measurements.";}

protected:
    bool checkState( Event) override;
    void doAction( Event) override;

private:
    Interactor::PathsInteractor::Ptr _pint;
    Vis::PathSetVisualisation *_vis;
};  // end class

}}   // end namespaces

#endif
