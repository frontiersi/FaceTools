/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#include <FaceModelViewer.h>
#include <Vis/BoundingVisualisation.h>
#include <Interactor/SelectNotifier.h>
#include <QStatusBar>
using IMode = QTools::InteractionMode;

namespace FaceTools { namespace Action {

class FaceTools_EXPORT ModelSelector
{
public:
    // Add the application's viewers to the ModelSelector.
    // Do this before calling any of the other functions.
    static void addViewer( FMV*, bool setDefault=false);

    // Optionally set a statically accessible status bar for convenient access.
    static void setStatusBar( QStatusBar*);
    static void showStatus( const QString&, int timeOutMilliSecs=0);    // No effect if status bar not set.
    static void clearStatus();

    // Set the cursor shape to be used across all viewers.
    static void setCursor( Qt::CursorShape);
    static void restoreCursor();

    // Return the interactor to connect to its onSelected event.
    // Call AFTER using addViewer to add all required viewers since the
    // construction of a ModelViewerInteractor derived type will call
    // ModelSelector::viewers() in its constructor.
    static const Interactor::SelectNotifier* selector();

    // Set/get the interaction mode for the viewers (camera - default, or actor).
    static void setInteractionMode( IMode, bool useCameraOffActor=false);
    static IMode interactionMode();

    // Return the viewer that the mouse was last over. Never returns null.
    // This is NOT necessarily the same as the currently selected FaceView's viewer!
    static FMV* mouseViewer();

    // Return the view the cursor is currently over (null if none).
    static Vis::FV* cursorView() { return sn().view();}

    // Return the prop the cursor is currently over (null if none).
    static const vtkProp* cursorProp() { return sn().prop();}

    // Return the viewer that the selected model is currently in. May be null!
    static FMV* selectedViewer() { return isViewSelected() ? selectedView()->viewer() : nullptr;}

    // Returns current mouse cursor position relative to the mouse viewer.
    static QPoint mousePos() { return mouseViewer()->mouseCoords();}

    // Simply returns the default viewer (set from addViewer).
    static FMV* defaultViewer();

    // Return all viewers in the order they were added.
    static const std::vector<FMV*>& viewers();

    static Vis::FV* selectedView();
    static bool isViewSelected() { return selectedView() != nullptr;}
    static FM* selectedModel() { return selectedView() ? selectedView()->data() : nullptr;}

    // Create a new FaceView instances and attach it to the given viewer.
    // If given viewer is null, FaceView added to the viewer of the currently
    // selected FaceView. If no FaceView is currently selected, the default viewer
    // is used. If no default viewer is set, the current mouseViewer() is used.
    // Failing all of that, the first available viewer set is used.
    // Returned FaceView will have been automatically selected.
    static Vis::FV* addFaceView( FM*, FMV *v=nullptr);

    // Programmatically select the given FaceView.
    static void setSelected( Vis::FV*);

    // Detach the FaceView from its viewer and delete it.
    static void removeFaceView( Vis::FV*);

    // Call removeFaceView for ALL associated FaceViews of the given FaceModel.
    static void remove( const FM*);

    // Set whether the camera autofocuses on the selected model upon selection (on by default).
    static void setAutoFocusOnSelectEnabled( bool);

    // Set whether bounding/orientation boxes are shown for the selected model (on by default).
    static void setShowBoundingBoxesOnSelected( bool);

    // Synchronise all bounding cuboid actors to match the model's orientation bounds.
    static void syncBoundingVisualisation( const FM*);

    // Provide a delegate function that will be executed on all open views and then updateRender (if set true).
    static void updateAllViews( const std::function<void(Vis::FV*)>&, bool updateRender=false);

    // Update rendering across all viewers.
    static void updateRender();

private:
    using Ptr = std::shared_ptr<ModelSelector>;
    static ModelSelector::Ptr _me;
    static Ptr me();
    static Interactor::SelectNotifier& sn();

    Vis::BoundingVisualisation _bvis;
    std::vector<FMV*> _viewers;
    QStatusBar* _sbar;
    bool _autoFocus;
    bool _showBoxes;
    int _defv;  // Default viewer index
    Interactor::SelectNotifier *_sn;

    void _doOnSelected( Vis::FV*, bool);
    ModelSelector();
    ModelSelector( const ModelSelector&) = delete;
    void operator=( const ModelSelector&) = delete;
};  // end class

}}   // end namespace

#endif
