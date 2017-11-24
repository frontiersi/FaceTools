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

namespace FaceTools
{

class FaceTools_EXPORT ModelSelector : public InteractionInterface
{ Q_OBJECT
public:
    ModelSelector( InteractiveModelViewer* viewer, Qt::Key key=Qt::Key_Control);
    ~ModelSelector() override;

    void setSelecting( bool);

signals:
    // Emitted on user double clicking in an empty area
    void onSelectAll();

    // Called as mouse pointer selects props as a result of double clicking and
    // dragging over props, or the user holding down the modifier key (Ctrl by default)
    // and single clicking on props.
    void onSelected( const vtkProp*, bool);

protected:
    void leftDoubleClick( const QPoint&) override;
    void mouseMove( const QPoint&) override;
    void leftButtonUp( const QPoint&) override;
    void leftButtonDown( const QPoint&) override;

private:
    InteractiveModelViewer *_viewer;
    QTools::KeyPressHandler *_kph;
    bool _selecting;
    bool _camLockState;
    bool _dblclick;
    boost::unordered_set<const vtkProp*> _selected;

    ModelSelector( const ModelSelector&);   // No copy
    void operator=( const ModelSelector&);  // No copy
};  // end class

}   // end namespace

#endif
