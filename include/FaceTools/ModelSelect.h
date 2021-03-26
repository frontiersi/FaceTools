/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_MODEL_SELECT_H
#define FACE_TOOLS_MODEL_SELECT_H

#include <FaceTools/Vis/FaceView.h>
#include <FaceTools/FaceModelViewer.h>
#include <FaceTools/Interactor/MouseHandler.h>
#include <rlib/MultiLock.h>
#include <QStatusBar>

namespace FaceTools {

using IMode = QTools::InteractionMode;

class FaceTools_EXPORT ModelSelect
{
public:
    // Add the application's viewers to the ModelSelect.
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
    // ModelSelect::viewers() in its constructor.
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

    // Return pointer to the other (non-selected) model if it exists.
    static FM* nonSelectedModel();

    // Return the selected model or null if not available.
    static FM::RPtr selectedModelScopedRead();
    static FM::WPtr selectedModelScopedWrite();

    // Return the model that *isn't* the selected one or null if not available.
    static FM::RPtr otherModelScopedRead();
    static FM::WPtr otherModelScopedWrite();

    // Return the model that *isn't* the given one or null if not available.
    static FM::RPtr otherModelScopedRead( const FM*);
    static FM::WPtr otherModelScopedWrite( const FM*);

    // Set the given FaceView as the selected one.
    static void setSelected( Vis::FV*);

    // Call to prevent changing the currently selected FaceView (unlocked if setSelected called).
    // Unlock by providing the returned key to unlockSelect. Key can only be used once!
    static int lockSelect();
    static void unlockSelect( int key);

    static Vis::FV* add( FM*, FMV*);
    static void remove( Vis::FV*);

    // Returns true iff the given FaceView still exists.
    static bool exists( const Vis::FV*);

    // Call removeFaceView for ALL associated FaceViews of the given FaceModel.
    static void remove( const FM*);

    // Call after making global changes - resets all model views.
    static void resetAllViews();

    // Update rendering across all viewers.
    static void updateRender();

private:
    using Ptr = std::shared_ptr<ModelSelect>;
    static ModelSelect::Ptr _me;
    static ModelSelect* me();
    static Interactor::SelectNotifier* _selectNotifier();
    static void _forceUnlockSelect();

    std::vector<FMV*> _viewers;
    QStatusBar* _sbar;
    int _defv;  // Default viewer index
    rlib::MultiLock _slocker;
    int _lockKey;
    Interactor::MouseHandler *_mouseHandler;

    ModelSelect();
    ModelSelect( const ModelSelect&) = delete;
    void operator=( const ModelSelect&) = delete;
};  // end class

}   // end namespace

#endif
