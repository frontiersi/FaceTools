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

#ifndef FACE_TOOLS_ACTION_VISUALISE_H
#define FACE_TOOLS_ACTION_VISUALISE_H

#include "FaceAction.h"
#include <BaseVisualisation.h>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionVisualise : public FaceAction
{ Q_OBJECT
public:
    // Change events specified in the provided VisualisationInterface are
    // combined with the standard set of events understood by this action.
    // Set isDefault to true if it is wished that the passed in visualisation
    // be the default applied to all newly loaded models.
    ActionVisualise( Vis::BaseVisualisation*, bool isDefault=false);

    QWidget* getWidget() const override { return _vint->getWidget();}

protected slots:
    bool testReady( FaceControl*) override;         // Applies default visualisation if none set yet.
    bool testChecked( FaceControl*) override;
    bool doAction( FaceControlSet&) override;       // Apply the visualisation.
    void respondTo( const FaceAction*, const ChangeEventSet*, FaceControl*) override;   // Update visualisation in response.
    void purge( const FaceControl*) override;       // Ensure that cached visualisation data is expunged.

private:
    Vis::BaseVisualisation *_vint; // The associated visualisation delegate
    const bool _isdefault;
};  // end class

}   // end namespace
}   // end namespace

#endif
