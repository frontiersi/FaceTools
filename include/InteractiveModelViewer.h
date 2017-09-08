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

#ifndef FACE_TOOLS_INTERACTIVE_MODEL_VIEWER_H
#define FACE_TOOLS_INTERACTIVE_MODEL_VIEWER_H

/**
 * Interactive version of ModelViewer.
 */

#include "ModelViewer.h"
#include "InteractionInterface.h"
#include <QLayout>

namespace FaceTools
{

class FaceTools_EXPORT InteractiveModelViewer : public QObject, public ModelViewer
{ Q_OBJECT
public:
    explicit InteractiveModelViewer( bool useFloodLights=true);
    explicit InteractiveModelViewer( QTools::VtkActorViewer*);
    virtual ~InteractiveModelViewer();

    void addToLayout( QLayout*);
    void removeFromLayout( QLayout*);
    void connectInterface( InteractionInterface*) const;

    // The most recent mouse coords with top-left origin.
    const QPoint& getMouseCoords() const { return _qinterface->getMouseCoords();}

    // Uses current mouse coords with top-left origin.
    const vtkProp* getPointedAt() const;

    void setCursor( QCursor);

signals:
    void requestContextMenu( const QPoint&);

private:
    ModelViewerQtInterface *_qinterface;
    void init();
};  // end class

}   // end namespace

#endif

