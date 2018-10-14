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

#ifndef FACE_TOOLS_MODEL_SELECT_INTERACTOR_H
#define FACE_TOOLS_MODEL_SELECT_INTERACTOR_H

/**
 * ModelSelectInteractor is an interaction handler that allows the user to select a single
 * FaceView with onSelected fired for (de)selection events. Select models with left or
 * right clicks.
 */

#include "ModelViewerInteractor.h"
#include <FaceViewSet.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT ModelSelectInteractor : public ModelViewerInteractor
{ Q_OBJECT
public:
    ModelSelectInteractor();

    void add( Vis::FV*);    // Adds to available for selection - does NOT fire onSelected.
    void remove( Vis::FV*); // Removes from available for selection and fires onSelected if FV was selected.

    void setSelected( Vis::FV*, bool);  // Mark given model as (de)selected and fire onSelected.

    inline bool isSelected( Vis::FV* fv) const { return fv == _selected;}
    inline bool isAvailable( Vis::FV* fv) const { return _available.has(fv);}

    const FVS& available() const { return _available;}   // Returns view/control instances available to select.
    Vis::FV* selected() const { return _selected;}

signals:
    void onSelected( Vis::FV*, bool);   // Fired selection / deselection of a model.

private:
    Vis::FV* _selected;
    FVS _available;

    bool leftButtonDown( const QPoint&) override;
    bool rightButtonDown( const QPoint&) override;
    bool leftDoubleClick( const QPoint&) override;
    void eraseSelected();
    Vis::FV* underPoint( const QPoint&) const;
};  // end class

}}   // end namespace

#endif
