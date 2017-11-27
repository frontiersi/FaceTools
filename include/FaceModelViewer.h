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

#ifndef FACE_TOOLS_FACE_MODEL_VIEWER_H
#define FACE_TOOLS_FACE_MODEL_VIEWER_H

#include "ActionFullScreenViewer.h"
#include "ModelSelector.h"
#include "FaceControl.h"

namespace FaceTools
{

class FaceTools_EXPORT FaceModelViewer : public QWidget
{ Q_OBJECT
public:
    FaceModelViewer( QMenu* contextMenu=NULL, QWidget *parent=NULL);
    ~FaceModelViewer() override;

    QAction* getFullScreenAction() { return _fsaction->qaction();}
    QAction* getScreenshotSaver() { return &_screenshotSaver;}
    QAction* getCameraResetter() { return &_cameraResetter;}
    QAction* getAxesToggler() { return &_axesToggler;}

    // Get the models from this widget into the given set returning the number
    // of models added. If the widget contains multiple views of the same model,
    // they are ignored (no model duplicates are allowed).
    size_t getModels( boost::unordered_set<FaceModel*>&) const;
    size_t getNumModels() const;       // Get number of models displayed

    // Get the user selected models into the given vector (if not NULL), returning #.
    size_t getSelectedModels( boost::unordered_set<FaceModel*> *umods=NULL) const;

    FaceControl* take( FaceModel*);
    bool give( FaceControl*);
    // Return pointer to the control if this has the model, otherwise returns NULL.
    FaceControl* get( FaceModel* fm) const;

    // Sets the given model (assumed to be accessible to this viewer) to be under
    // the control of this viewer or not. Returns false if the given model is
    // not owned by this viewer. By default, causes updateSelected to be emitted
    // on success - prevent this from happening by setting emitUpdateSelected to false.
    bool setControlled( FaceModel*, bool controlled=true, bool emitUpdateSelected=true);

    void updateRender();    // Calls updateRender on viewer
    void addKeyPressHandler( QTools::KeyPressHandler*);
    void removeKeyPressHandler( QTools::KeyPressHandler*);

    void applyOptions( const ModelOptions&);

    cv::Mat_<cv::Vec3b> grabImage() const; // Copy out the current screen.

signals:
    void updatedSelected( FaceControl*, bool);  // Notify that user has (de)selected the given model.
    void toggleZeroArea( bool); // When going from positve to zero viewing area (true) and back (false).

protected:
    void resizeEvent( QResizeEvent*) override;

private slots:
    void doOnSelected( FaceControl*, bool);
    void showContextMenu( const QPoint&);

private:
    QMenu *_cmenu;
    InteractiveModelViewer *_viewer;
    ModelSelector *_selector;
    ActionFullScreenViewer *_fsaction;
    QAction _screenshotSaver, _cameraResetter, _axesToggler;
    boost::unordered_map<FaceModel*, FaceControl*> _modelConts;

    FaceModelViewer( const FaceModelViewer&); // No copy
    void operator=( const FaceModelViewer&);  // No copy
};  // end class

}   // end namespace

#endif

