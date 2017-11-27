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

#ifndef FACE_TOOLS_MODEL_SELECTOR_H
#define FACE_TOOLS_MODEL_SELECTOR_H

#include "InteractiveModelViewer.h"
#include "FaceControl.h"

namespace FaceTools
{

class FaceTools_EXPORT ModelSelector : public InteractionInterface
{ Q_OBJECT
public:
    explicit ModelSelector( InteractiveModelViewer* viewer);

    void add( FaceControl*);    // Makes selected
    void remove( FaceControl*); // Makes deselected
    void setSelected( FaceControl*, bool);  // Mark fcont as selected or not - does NOT cause onSelected to fire!

    const boost::unordered_set<FaceControl*>& getSelected() const { return _selected;}

signals:
    void onSelected( FaceControl*, bool);

protected:
    void rightButtonDown( const QPoint&) override;
    void leftDoubleClick( const QPoint&) override;

private:
    InteractiveModelViewer *_viewer;
    boost::unordered_set<FaceControl*> _selected;
    boost::unordered_set<FaceControl*> _available;

    FaceControl* findFromProp( const vtkProp* p) const;
    ModelSelector( const ModelSelector&);   // No copy
    void operator=( const ModelSelector&);  // No copy
};  // end class

}   // end namespace

#endif
