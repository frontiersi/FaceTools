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

#ifndef FACE_TOOLS_ACTION_SELECT_H
#define FACE_TOOLS_ACTION_SELECT_H

/**
 * Handles FaceControl selections programmatically and via internal interactor.
 * Responds to setSelected by showing or hiding outlines around selected FaceControls.
 */

#include "FaceAction.h"
#include <CuboidView.h>
#include <ModelSelectInteractor.h>
#include <unordered_map>

namespace FaceTools {
namespace Action {

class FaceTools_EXPORT ActionSelect : public FaceAction
{ Q_OBJECT
public:
    ActionSelect();

    void setViewer( FaceModelViewer* v);    // Set initial viewer.
    FaceModelViewer* viewer() const;        // Viewer the interactor is currently attached to.

    const FaceControlSet& selected() const { return _selector.selected();}
    const FaceControlSet& available() const { return _selector.available();}

    // Programmatically cause onSelect events to fire.
    void addFaceControl( FaceControl* fc) { _selector.add(fc);}
    void removeFaceControl( FaceControl* fc) { _selector.remove(fc); emit onRemove(fc);}
    void select( FaceControl*, bool);

    // Enable/disable the ability to select
    void setSelectEnabled(bool);

signals:
    void onSelect( FaceControl*, bool);
    void onRemove( FaceControl*);

protected slots:
    // Recheck selection visualisation for the given FaceControl instance (which may have changed).
    void respondTo( const FaceAction*, const ChangeEventSet*, FaceControl*) override;

private slots:
    void doOnSelected( FaceControl*, bool);

private:
    Interactor::ModelSelectInteractor _selector;
    std::unordered_map<FaceModel*, Vis::CuboidView*> _outlines;  // Per model selection outlines.
    void displaySelected( const FaceControl*, bool);
};  // end class

}   // end namespace
}   // end namespace

#endif
