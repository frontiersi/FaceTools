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

#ifndef FACE_TOOLS_ACTION_REMESH_H
#define FACE_TOOLS_ACTION_REMESH_H

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionRemesh : public FaceAction
{ Q_OBJECT
public:
    ActionRemesh( const QString& dname="Remesh", const QIcon& icon=QIcon(), QProgressBar* pb=NULL);   // Async if pb not NULL

    double maxEdgeLength() const { return _maxEdgeLen;}

private slots:
    bool testEnabled() const override { return readyCount() == 1;}   // Only enabled for single selections
    void doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool) override { cs.insert(GEOMETRY_CHANGE);}
    bool doAction( FaceControlSet&) override;

private:
    double _maxEdgeLen;
};  // end class

}   // end namespace
}   // end namespace

#endif
