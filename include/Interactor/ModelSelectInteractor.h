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

#ifndef FACE_TOOLS_MODEL_SELECT_INTERACTOR_H
#define FACE_TOOLS_MODEL_SELECT_INTERACTOR_H

/**
 * ModelSelectInteractor is an interaction handler that allows the user to select single or
 * multiple FaceControls with onSelected fired for (de)selection events. Select models with
 * a right click or a double left click. Model selection can be temporarily enabled/disabled
 * with enableSelect (default true).
 */

#include "ModelViewerInteractor.h"
#include <FaceControlSet.h>

namespace FaceTools {
namespace Interactor {

class FaceTools_EXPORT ModelSelectInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    explicit ModelSelectInteractor( bool exclusiveSelect=false);

    // Enable/disable user interaction (de)selection of models.
    // Programmatic selection of models (e.g. using setSelected) NOT AFFECTED.
    void enableUserSelect( bool);

    // Initially a multi-model selector, call setExclusiveSelect(true) to set models to
    // be selected exclusively. Exclusivity is enforced on subsequent model selects.
    void setExclusiveSelect( bool);
    bool isExclusiveSelect() const;

    void add( FaceControl*);    // Adds to available for selection and fires onSelected.
    void remove( FaceControl*); // Removes from available for selection and fires onSelected.

    void setSelected( FaceControl*, bool);  // Mark given model as (de)selected and fire onSelected.

    bool isSelected( FaceControl*) const;   // Returns true iff model is selected.
    bool isAvailable( FaceControl*) const;  // Returns true iff model was added and not yet removed.

    const FaceControlSet& selected() const { return _selected;}     // Returns selected view/control instances.
    const FaceControlSet& available() const { return _available;}   // Returns view/control instances available to select.

signals:
    void onSelected( FaceControl*, bool);   // Fired selection / deselection of a model.

private:
    bool _exclusive;
    bool _enabled;
    FaceControlSet _selected;
    FaceControlSet _available;

    bool rightButtonDown( const QPoint&) override;    // Overridden to fire onSelected and onRightButtonDown
    bool leftDoubleClick( const QPoint&) override;    // Overridden to fire onSelected
    void deselectAll();
    void insertSelected( FaceControl*);
    void eraseSelected( FaceControl*);
};  // end class

}   // end namespace
}   // end namespace

#endif
