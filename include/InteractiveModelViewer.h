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
    explicit InteractiveModelViewer( QTools::VtkActorViewer*);
    virtual ~InteractiveModelViewer(){}

    void addToLayout( QLayout*);
    void removeFromLayout( QLayout*);

    // Set the provided InteractionInterface ii to receive
    // inputs from this viewer's ModelViewerQtInterface,
    // replacing the previously set input interface.
    void connectInterface( InteractionInterface*) const;

    // Disconnect the given interaction interface from this viewer.
    void disconnectInterface( InteractionInterface*) const;

    void setCameraLocked( bool v);
    bool isCameraLocked() const;

    // The most recent mouse coords with top-left origin.
    const QPoint& getMouseCoords() const { return _qinterface->getMouseCoords();}

    const vtkProp* getPointedAt() const; // Uses current mouse coords with top-left origin.

    void setCursor( QCursor);

    void addKeyPressHandler( QTools::KeyPressHandler*);
    void removeKeyPressHandler( QTools::KeyPressHandler*);

signals:
    void requestContextMenu( const QPoint&);

public slots:
    void setFullScreen( bool enable=true);

private:
    ModelViewerQtInterface *_qinterface;
    QWidget* _pwidget;
    Qt::WindowFlags _winflags;
    void init();
};  // end class

}   // end namespace

#endif

