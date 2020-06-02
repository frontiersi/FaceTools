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

#ifndef FACE_TOOLS_ACTION_EXTRACT_FACE_H
#define FACE_TOOLS_ACTION_EXTRACT_FACE_H

#include "FaceAction.h"
#include <FaceTools/Interactor/RadialSelectHandler.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExtractFace : public FaceAction
{ Q_OBJECT
public:
    ActionExtractFace( const QString&, const QIcon&, const Interactor::RadialSelectHandler&);

    QString toolTip() const override;
    QString whatsThis() const override;

    // Facial radius default is 100 units (mm) and minimum possible is 1 mm.
    static void setCropRadius( float d);
    static float cropRadius();

    // Extract the facial region as all points within d units of the identified position
    // and on return set out parameter T (if not null) to the identified transform of the
    // face (add inverse of this to transform to standard position).
    static r3d::Mesh::Ptr extractFacialRegion( const FM*, float d, Mat4f* T=nullptr);

    static r3d::Mesh::Ptr cropRegion( const FM*, const IntSet &fids);

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    const Interactor::RadialSelectHandler &_handler;
    r3d::Mesh::Ptr _nmod;
    static float s_cropRadius;
};  // end class

}}   // end namespace

#endif
