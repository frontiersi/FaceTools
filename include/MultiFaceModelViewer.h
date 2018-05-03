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
#include "FaceControlSet.h"
#include <QToolButton>
#include <QHBoxLayout>
#include <QSplitter>

namespace FaceTools {

class FaceTools_EXPORT MultiFaceModelViewer : public QWidget
{ Q_OBJECT
public:
    explicit MultiFaceModelViewer( QWidget *parent=NULL);
    ~MultiFaceModelViewer() override;

public slots:
    void insert( FaceModel*);   // Ensures model is shown in the centre panel and emits onSelected.
    void remove( FaceModel*);   // Remove all views for the given model and does NOT emit onSelected.

    // Toggle visibility of left/right viewers
    void setLeftViewerVisible(bool);
    void setRightViewerVisible(bool);

    void saveScreenshot() const;    // Horizontally concatenates images from the three viewers

signals:
    void onSelected( FaceControl*, bool);   // Connect to FaceActionManager::setSelected
    void onRemoving( FaceControl*);         // Connect to FaceActionManager::remove

private slots:
    void moveLeftToCentre();
    void moveCentreToLeft();
    void moveCentreToRight();
    void moveRightToCentre();

    void copyLeftToCentre();
    void copyCentreToLeft();
    void copyCentreToRight();
    void copyRightToCentre();

    void doOnLeftViewerSetSelected( FaceControl*, bool);
    void doOnCentreViewerSetSelected( FaceControl*, bool);
    void doOnRightViewerSetSelected( FaceControl*, bool);

private:
    QAction* _moveLeftToCentreAction;
    QAction* _copyLeftToCentreAction;
    QAction* _moveRightToCentreAction;
    QAction* _copyRightToCentreAction;

    QAction* _moveCentreToLeftAction;
    QAction* _copyCentreToLeftAction;
    QAction* _moveCentreToRightAction;
    QAction* _copyCentreToRightAction;

    FaceModelViewer *_v0;
    FaceModelViewer *_v1;
    FaceModelViewer *_v2;
    QSplitter *_splitter;
    FaceControlSet _fconts;

    void moveViews( FaceModelViewer*, FaceModelViewer*);
    void copyViews( FaceModelViewer*, FaceModelViewer*);
    void removeViews( FaceModelViewer*);

    void checkEnableLeftToCentre();
    void checkEnableCentreToLeft();
    void checkEnableCentreToRight();
    void checkEnableRightToCentre();
    size_t canCopyTo( FaceModelViewer*, FaceModelViewer*, FaceControlSet *fcs=NULL) const;

    bool deleteFaceControl( FaceControl*);
    void activateAllOnViewer( FaceModelViewer*, bool);
    void activateOnViewer( FaceModelViewer*, FaceControl*, bool);
    void addCommonButtons( QLayout*, FaceModelViewer*);
    QToolButton* makeButton( QAction*);

    MultiFaceModelViewer( const MultiFaceModelViewer&); // No copy
    void operator=( const MultiFaceModelViewer&);       // No copy
};  // end class

}   // end namespace

#endif
