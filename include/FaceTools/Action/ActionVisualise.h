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

#ifndef FACE_TOOLS_ACTION_VISUALISE_H
#define FACE_TOOLS_ACTION_VISUALISE_H

#include "FaceAction.h"
#include <FaceTools/Vis/BaseVisualisation.h>

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionVisualise : public FaceAction
{ Q_OBJECT
public:
    ActionVisualise( const QString&, const QIcon&, Vis::BaseVisualisation*, const QKeySequence& ks=QKeySequence());

    Vis::BaseVisualisation* visualisation() { return _vis;}

protected:
    void purge( const FM*) override;

    // update returns true iff a view is selected and the view currently
    // has this visualisation visible on it. If there are other factors than
    // can cause the visualisation to become visible, they should be checked
    // within an overridden version of this function.
    bool update( Event) override;

    // isAllowed returns true iff isChecked() returns true or the
    // visualisation is available for the currently selected view.
    bool isAllowed( Event) override;

    void doAction( Event) override;
    Event doAfterAction( Event) override;

private:
    void _toggleVis( Vis::FV*, const QPoint*) const;
    bool _isVisAvailable( const Vis::FV*) const;
    FVS _getWorkViews();
    Vis::BaseVisualisation *_vis; // The visualisation delegate
    Event _evg;
};  // end class

}}   // end namespaces

#endif
