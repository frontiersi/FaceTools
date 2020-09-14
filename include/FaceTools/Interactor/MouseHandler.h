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

#ifndef FACE_TOOLS_MOUSE_HANDLER_H
#define FACE_TOOLS_MOUSE_HANDLER_H

#include <FaceTools/FaceTypes.h>
#include <QTools/VtkMouseHandler.h>
#include <vtkRenderWindow.h>
#include <type_traits>

namespace FaceTools { namespace Interactor {

class GizmoHandler;
class SelectNotifier;

class FaceTools_EXPORT MouseHandler : public QTools::VMH
{
public:
    MouseHandler();
    ~MouseHandler() override;

    void addViewer( FMV*, bool isDefault=false);

    inline FMV* mouseViewer() const { return _vwr;}     // Viewer pointer was last over (never null).
    inline const vtkProp *prop() const { return _pnow;} // Prop cursor is over (may be null).

    // Get notifier for connecting to model selection events.
    inline SelectNotifier* selectNotifier() { return _snot;}

    void refreshHandlers();

    // Register a handler for mouse events and model/prop enter/leave events.
    // The order of addition determines the order in which the registered handlers
    // are called. If an earlier handler swallows the event (returns true from its
    // respective event handling function), later handlers are ignored.
    // Returns the index of the added handler.
    int registerHandler( GizmoHandler*);

    // Call after all handlers have been registered to call their respective postRegister functions.
    void finishRegistration();

    template <class T>
    T* handler() const
    {
        static_assert( std::is_base_of<GizmoHandler, T>::value, "Requested handler must derive from GizmoHandler!");
        T *rgh = nullptr;
        for ( GizmoHandler *gh : _gizmos)
            if ( (rgh = qobject_cast<T*>(gh)))
                break;
        assert( rgh);
        return rgh;
    }   // end handler

private:
    bool mouseMove() override;

    bool leftButtonDown() override; // Not called if leftDoubleClick
    bool leftButtonUp() override;   // Not called if leftDoubleClick
    bool leftDoubleClick() override;

    bool middleButtonDown() override;
    bool middleButtonUp() override;

    bool rightButtonDown() override;
    bool rightButtonUp() override;
    bool rightDoubleClick() override;

    bool leftDrag() override;  // Move mouse with left button depressed.
    bool rightDrag() override; // Move mouse with right button depressed.
    bool middleDrag() override;// Move mouse with middle button depressed.

    bool mouseWheelForward() override;
    bool mouseWheelBackward() override;

    void mouseEnter( const QTools::VtkActorViewer*) override;
    std::unordered_map<const vtkRenderWindow*, FMV*> _vwrs;

    std::vector<GizmoHandler*> _gizmos;

    SelectNotifier *_snot;
    FMV *_vwr;
    Vis::FV *_mnow;
    Vis::FV *_mnxt;
    const vtkProp *_pnow;
    const vtkProp *_pnxt;
  
    void _setPointedAt();
    void _testMouseCursor();
    Vis::FV* _selectView( Vis::FV*) const;
    bool _handleEvent( const std::function<bool(GizmoHandler*)>&) const;
    MouseHandler( const MouseHandler&) = delete;
    void operator=( const MouseHandler&) = delete;
};  // end class

}}   // end namespace

#endif
