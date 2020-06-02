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

#ifndef FACE_TOOLS_ACTION_ALIGN_MODEL_H
#define FACE_TOOLS_ACTION_ALIGN_MODEL_H

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionAlignModel : public FaceAction
{ Q_OBJECT
public:
    ActionAlignModel( const QString&, const QIcon&, const QKeySequence &ks=QKeySequence());

    QString toolTip() const override { return "Align the selected face into centred and upright position.";}

    QString whatsThis() const override;

    // Calculate and return the matrix such that its inverse when applied
    // to the given model will cause it to be aligned in standard position.
    // If useMask is true, the returned alignment transform will be the
    // inverse of the mask's current transform matrix (if it exists)
    // otherwise a new alignment matrix will be calculated based on
    // texture (if available) or geometry.
    static Mat4f calcAlignmentTransform( const FM*, bool useMask=true);

protected:
    bool isAllowed( Event) override;
    bool doBeforeAction( Event) override;
    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    Event _egrp;
};  // end class

}}   // end namespace

#endif
