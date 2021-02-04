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

#ifndef FACE_TOOLS_ACTION_EXTRACT_FACE_H
#define FACE_TOOLS_ACTION_EXTRACT_FACE_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionExtractFace : public FaceAction
{ Q_OBJECT
public:
    ActionExtractFace( const QString&, const QIcon&);

    QString toolTip() const override;
    QString whatsThis() const override;

    // Facial radius default is 100 units (mm) and minimum possible is 1 mm.
    static void setCropRadius( float d);
    static float cropRadius();

    // Extract the facial region as all points within cropRadius of the discovered front.
    // This function will also align the given model into standard position.
    static r3d::Mesh::Ptr extract( FM&);

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    Event _ev;
    static float s_cropRadius;
};  // end class

}}   // end namespace

#endif
