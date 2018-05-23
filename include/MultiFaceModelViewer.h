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
#include <ModelViewerEntryExitInteractor.h>
#include <ActionSelect.h>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSplitter>

namespace FaceTools {

class FaceTools_EXPORT MultiFaceModelViewer : public QWidget
{ Q_OBJECT
public:
    MultiFaceModelViewer( Action::ActionSelect* selector, QWidget *parent=NULL);
    ~MultiFaceModelViewer() override;

    FaceModelViewer* leftViewer() { return _v0;}
    FaceModelViewer* centreViewer() { return _v1;}
    FaceModelViewer* rightViewer() { return _v2;}

public slots:
    void remove( FaceModel*);   // Remove all views for the given model

    // Toggle visibility of left/right viewers
    void setLeftViewerVisible(bool);
    void setRightViewerVisible(bool);

    void saveScreenshot() const;    // Horizontally concatenates images from the three viewers

private slots:
    void moveLeftToCentre();
    void moveCentreToLeft();
    void moveCentreToRight();
    void moveRightToCentre();

    void copyLeftToCentre();
    void copyCentreToLeft();
    void copyCentreToRight();
    void copyRightToCentre();

    void doOnViewerEntered();
    void doOnSelect( FaceControl*, bool);

private:
    Action::ActionSelect *_selector;

    // Fixed ModelViewer interactors.
    Interactor::ModelViewerEntryExitInteractor *_i0;
    Interactor::ModelViewerEntryExitInteractor *_i1;
    Interactor::ModelViewerEntryExitInteractor *_i2;

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

    void moveViews( FaceModelViewer*);
    void copyViews( FaceModelViewer*);
    void removeViews( FaceModelViewer*);

    void checkEnableLeftToCentre();
    void checkEnableCentreToLeft();
    void checkEnableCentreToRight();
    void checkEnableRightToCentre();
    bool canMoveFrom( FaceModelViewer*) const;
    bool canCopyTo( FaceModelViewer*) const;

    bool deleteFaceControl( FaceControl*);

    void addCommonButtons( QLayout*, FaceModelViewer*);
    QToolButton* makeButton( QAction*);

    MultiFaceModelViewer( const MultiFaceModelViewer&); // No copy
    void operator=( const MultiFaceModelViewer&);       // No copy
};  // end class

}   // end namespace

#endif
