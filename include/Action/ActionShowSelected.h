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

#ifndef FACE_TOOLS_ACTION_SHOW_SELECTED_H
#define FACE_TOOLS_ACTION_SHOW_SELECTED_H

/**
 * Responds to setSelected by showing or hiding a boundary around the given FaceControl.
 * Does not implement doAction().
 */

#include "FaceAction.h"
#include <OutlinesView.h>
#include <unordered_map>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionShowSelected : public FaceAction
{ Q_OBJECT
public:
    ActionShowSelected();

protected slots:
    void tellSelected( FaceControl*, bool) override;   // Show or hide outline around given FaceControl

    // Recheck selection visualisation for the given FaceControl instance (which may have changed).
    void respondToChange( FaceControl*) override;

private:
    std::unordered_map<FaceModel*, Vis::OutlinesView*> _outlines;  // Per model selection outlines.
};  // end class

}   // end namespace
}   // end namespace

#endif
