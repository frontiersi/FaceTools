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

#ifndef FACE_TOOLS_ACTION_FIX_NORMALS_H
#define FACE_TOOLS_ACTION_FIX_NORMALS_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionFixNormals : public FaceAction
{ Q_OBJECT
public:
    ActionFixNormals( const QString&, const QIcon&);

    QString toolTip() const override { return "Fixes inconsistencies in face normals so that they all agree on the inside/outside.";}

    // Tries to make a consistent ordering of face normals on the given model.
    // The absolute value of the returned value gives the number of faces fixed (vertex ordering reversed).
    // If the returned value is negative, then some manifold on the mesh was found to be twisted
    // (e.g. like a mobious strip) and a mesh wide consistent ordering of face normals was not possible.
    static int fixNormals( FM*);

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    int _nfixed;
};  // end class

}}   // end namespace

#endif
