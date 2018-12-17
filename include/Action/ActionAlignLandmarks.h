/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_ACTION_ALIGN_LANDMARKS_H
#define FACE_TOOLS_ACTION_ALIGN_LANDMARKS_H

/**
 * Aligns models with landmarks sharing the same names using Procrustes superimposition.
 * Requires a minimum of three landmarks with same names.
 */

#include "FaceAction.h"

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionAlignLandmarks : public FaceAction
{ Q_OBJECT
public:
    // Async if pb not NULL
    ActionAlignLandmarks( const QString& dname="Align Landmarks", const QIcon& icon=QIcon(), QProgressBar* pb=nullptr);

protected slots:
    bool testEnabled( const QPoint*) const override;
    bool doAction( FVS&, const QPoint&) override;
    void doAfterAction( EventSet& cs, const FVS&, bool) override { cs.insert(AFFINE_CHANGE);}
};  // end class

} }   // end namespace

#endif
