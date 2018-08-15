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

#ifndef FACE_TOOLS_MULTI_FACE_MODEL_VIEWER_H
#define FACE_TOOLS_MULTI_FACE_MODEL_VIEWER_H

#include "FaceModelViewer.h"
#include <QToolButton>
#include <QHBoxLayout>
#include <QSplitter>

namespace FaceTools {

class FaceTools_EXPORT MultiFaceModelViewer : public QWidget
{ Q_OBJECT
public:
    MultiFaceModelViewer( QWidget *parent=nullptr);
    ~MultiFaceModelViewer() override;

    FaceModelViewer* leftViewer() { return _v0;}
    FaceModelViewer* centreViewer() { return _v1;}
    FaceModelViewer* rightViewer() { return _v2;}

    void setCopyLeftToCentreAction( QAction*);
    void setMoveLeftToCentreAction( QAction*);
    void setMoveCentreToLeftAction( QAction*);
    void setCopyCentreToLeftAction( QAction*);
    void setCopyCentreToRightAction( QAction*);
    void setMoveCentreToRightAction( QAction*);
    void setMoveRightToCentreAction( QAction*);
    void setCopyRightToCentreAction( QAction*);

    void setLeftResetCameraAction( QAction*);
    void setCentreResetCameraAction( QAction*);
    void setRightResetCameraAction( QAction*);

    void setLeftSaveImageAction( QAction*);
    void setCentreSaveImageAction( QAction*);
    void setRightSaveImageAction( QAction*);

public slots:
    void setViewerVisible(int, bool);

private:
    std::vector<QToolButton*> _copyButton;
    std::vector<QToolButton*> _moveButton;
    std::vector<QToolButton*> _resetCameraButtons;
    std::vector<QToolButton*> _saveImageButtons;

    FaceModelViewer *_v0;
    FaceModelViewer *_v1;
    FaceModelViewer *_v2;
    QSplitter *_splitter;

    void addCommonButtons( QLayout*);

    MultiFaceModelViewer( const MultiFaceModelViewer&) = delete;
    void operator=( const MultiFaceModelViewer&) = delete;
};  // end class

}   // end namespace

#endif
