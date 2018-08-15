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

#ifndef FACE_TOOLS_COPY_VIEWER_H
#define FACE_TOOLS_COPY_VIEWER_H

/**
 * Copys the currently selected FaceControl instances to the target viewer.
 */

#include "FaceAction.h"
#include <ModelSelector.h>
#include <FaceModelViewer.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionCopyViewer : public FaceAction
{ Q_OBJECT
public:
    // If a source viewer is set (can be NULL), this action will only be enabled for
    // the subset of the selected FaceControls that are in the source viewer.
    ActionCopyViewer( FaceModelViewer* targetViewer, ModelSelector*,
                      FaceModelViewer* sourceViewer,
                      const QString&, const QIcon&);

protected slots:
    bool testReady( const FaceControl* fc) override;
    bool doAction( FaceControlSet&, const QPoint&) override;
    void doAfterAction( ChangeEventSet& cs, const FaceControlSet&, bool) override { cs.insert(VIEW_CHANGE);}
    bool displayDebugStatusProgression() const override { return false;}

private:
    FaceModelViewer *_tviewer;
    ModelSelector *_selector;
    FaceModelViewer *_sviewer;
};  // end class

}   // end namespace
}   // end namespace

#endif
