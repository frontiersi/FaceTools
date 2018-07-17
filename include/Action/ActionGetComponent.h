/************************************************************************
 * Copyright (C) 2017 Richard Palmer
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

#ifndef FACE_TOOLS_ACTION_GET_COMPONENT_H
#define FACE_TOOLS_ACTION_GET_COMPONENT_H

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionGetComponent : public FaceAction
{ Q_OBJECT
public:
    ActionGetComponent( const QString& dname="Get Face Component", const QIcon& icon=QIcon(), QProgressBar* pb=NULL);

    // TODO Currently requires the nose landmark to get the face but should make generic for any component.

private slots:
    bool testReady( const FaceControl*) override;
    bool testEnabled() const override { return readyCount() == 1;}
    bool doAction( FaceControlSet&) override;
    void doAfterAction( ChangeEventSet&, const FaceControlSet&, bool) override;
};  // end class

}   // end namespace
}   // end namespace

#endif
