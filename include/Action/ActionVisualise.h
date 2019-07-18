/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ActionVisualise : public FaceAction
{ Q_OBJECT
public:
    ActionVisualise( const QString&, const QIcon&, Vis::BaseVisualisation*, const QKeySequence& ks=QKeySequence());

    QWidget* widget() const override { return _vis->widget();}

    Vis::BaseVisualisation* visualisation() { return _vis;}

    bool isExclusive() const { return !_vis->isToggled() || _vis->isExclusive();}

protected:
    void purge( const FM*, Event) override;

    // checkState returns true iff a view is selected and the view currently
    // has this visualisation visible on it. If there are other factors than
    // can cause the visualisation to become visible, they should be looked at
    // within an overridden version of this function.
    bool checkState( Event) override;

    // checkEnable returns true iff isChecked() returns true or the
    // visualisation is available for the currently selected view.
    bool checkEnable( Event) override;

    void doAction( Event) override;

private:
    bool toggleVis( Vis::FV*, const QPoint*);
    Vis::BaseVisualisation *_vis; // The visualisation delegate

    bool isVisAvailable( const Vis::FV*) const;
};  // end class

}}   // end namespaces

#endif
