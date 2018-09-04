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

#ifndef FACE_TOOLS_ACTION_SAVE_SCREENSHOT_H
#define FACE_TOOLS_ACTION_SAVE_SCREENSHOT_H

/**
 * Save as image snapshots the renderers for the actioned actioned Vis::FVs.
 * If multiple viewers are added, the screenshot is generated from a horizontal
 * concatenation of the images from the added viewers (left to right in additive order).
 */

#include "FaceAction.h"

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionSaveScreenshot : public FaceAction
{ Q_OBJECT
public:
    // If no viewers are added, screenshots are saved from viewers of the FVS passed in to doAction.
    ActionSaveScreenshot( const QString& dname, const QIcon& icon, FMV* mv=nullptr);

    void addViewer( FMV* v) { _viewers.insert(v);}
   
public slots:
    bool doAction( FVS&, const QPoint&) override;
    bool displayDebugStatusProgression() const override { return false;}

private:
    FMVS _viewers;
};  // end class

}   // end namespace
}   // end namespace

#endif
