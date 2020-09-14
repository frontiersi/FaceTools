/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#include <FaceTools/FaceModelViewer.h>
#include <FaceTools/Vis/BoundingVisualisation.h>
#include <FaceTools/Interactor/MouseHandler.h>
#include <QStatusBar>

namespace FaceTools { namespace Action {

using IMode = QTools::InteractionMode;

class FaceTools_EXPORT ModelSelector
{
public:
    // Add the application's viewers to the ModelSelector.
    // Do this before calling any of the other functions.
    static void addViewer( FMV*, bool setDefault=false);

    // Optionally set a statically accessible status bar for convenient access.
    static void setStatusBar( QStatusBar*);
    static void showStatus( const QString&, int timeOutMilliSecs=0, bool repaintNow=false);    // No effect if status bar not set.
    static QString currentStatus();
    static void clearStatus();

    // Set the cursor shape to be used across all viewers.
    static void setCursor( Qt::CursorShape);
    static void restoreCursor();

    // Return the interactor to connect to its onSelected event.
    // Call AFTER using addViewer to add all required viewers since the
    // construction of a ModelViewerInteractor derived type will call
    // ModelSelector::viewers() in its constructor.
    static const Interactor::SelectNotifier* selectNotifier();

    // Register an interaction handler (order of registration matters).
    static void registerHandler( Interactor::GizmoHandler*);

    // Call after all handlers have been registered (some may depend on others).
    static void finishRegisteringHandlers() { me()->_mouseHandler->finishRegistration();}

    // Return the first registered handler of the given type or null if not registered.
    template <class T>
    static T* handler() { return me()->_mouseHandler->handler<T>();}

    // Cause handlers to refresh their own states.
    static void refreshHandlers();

    // Set/get the interaction mode for the viewers (camera - default, or actor).
    static void setInteractionMode( IMode);
    static IMode interactionMode();

    // Return the viewer that the mouse was last over. Never returns null.
    // This is NOT necessarily the same as the currently selected FaceView's viewer!
    static FMV* mouseViewer();

    // Return the prop the cursor is currently over (null if none).
    static const vtkProp* cursorProp() { return me()->_mouseHandler->prop();}

    // Return the viewer that the selected model is currently in. May be null!
    static FMV* selectedViewer() { return isViewSelected() ? selectedView()->viewer() : nullptr;}

    // Simply returns the default viewer (set from addViewer).
    static FMV* defaultViewer();

    // Return all viewers in the order they were added.
    static const std::vector<FMV*>& viewers();

    // Set the view angle for all viewers.
    static void setViewAngle( double);

    static Vis::FV* selectedView();
    static bool isViewSelected() { return selectedView() != nullptr;}
    static FM* selectedModel() { return selectedView() ? selectedView()->data() : nullptr;}

    // Programmatically select the given FaceView.
    static void setSelected( Vis::FV*);

    // Call with true to prevent changes to currently selected FaceView (unlocked if setSelected called).
    static void setLockSelected( bool);

    static Vis::FV* add( FM*, FMV*);
    static void remove( Vis::FV*);

    // Call removeFaceView for ALL associated FaceViews of the given FaceModel.
    static void remove( const FM*);

    // Set whether the camera autofocuses on the selected model upon selection (on by default).
    static void setAutoFocusOnSelectEnabled( bool);

    // Set whether bounding/orientation boxes are shown for the selected model (on by default).
    static void setShowBoundingBoxesOnSelected( bool);

    // Synchronise all bounding cuboid actors to match the model's orientation bounds.
    static void syncBoundingVisualisation( const FM*);

    // Provide a delegate that will be executed on all open views and then updateRender (if set true).
    static void updateAllViews( const std::function<void(Vis::FV*)>&, bool updateRender=false);

    // Update rendering across all viewers.
    static void updateRender();

private:
    using Ptr = std::shared_ptr<ModelSelector>;
    static ModelSelector::Ptr _me;
    static Ptr me();
    static Interactor::SelectNotifier* _selectNotifier();

    Vis::BoundingVisualisation _bvis;
    std::vector<FMV*> _viewers;
    QStatusBar* _sbar;
    bool _autoFocus;
    bool _showBoxes;
    int _defv;  // Default viewer index
    int _lockCount;
    Interactor::MouseHandler *_mouseHandler;

    void _doOnSelected( Vis::FV*, bool);
    ModelSelector();
    ModelSelector( const ModelSelector&) = delete;
    void operator=( const ModelSelector&) = delete;
};  // end class

}}   // end namespace

#endif
