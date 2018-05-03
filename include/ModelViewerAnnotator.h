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

#ifndef FACE_TOOLS_MODEL_VIEWER_ANNOTATOR_H
#define FACE_TOOLS_MODEL_VIEWER_ANNOTATOR_H

#include "FaceTools_Export.h"
#include <string>
#include <unordered_map>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkTextActor.h>
#include <QObject>

namespace FaceTools {

class FaceTools_EXPORT ModelViewerAnnotator : public QObject
{ Q_OBJECT
public:
    explicit ModelViewerAnnotator( vtkRenderer*);
    virtual ~ModelViewerAnnotator();

    enum TextJustification
    {
        LeftJustify,
        CentreJustify,
        RightJustify
    };  // end enum

    // Show message text at given screen proportional position returning its ID.
    int showMessage( float colPos, float rowPos, TextJustification justification, const std::string& msg);
    bool removeMessage( int msgID);

public slots:
    void doOnUpdateMessagePositions();

private:
    vtkSmartPointer<vtkRenderer> _renderer;
    struct Message;
    std::unordered_map<int, Message*> _messages;
};  // end class

}   // end namespace

#endif
