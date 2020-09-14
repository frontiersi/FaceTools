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

#include <Interactor/MouseHandler.h>
#include <Interactor/GizmoHandler.h>
#include <Interactor/SelectNotifier.h>
#include <FaceModelViewer.h>
#include <FaceModel.h>
#include <cassert>
using FaceTools::Interactor::MouseHandler;
using FaceTools::Interactor::GizmoHandler;
using FaceTools::Interactor::SelectNotifier;
using FaceTools::Vis::FV;
using FaceTools::FMV;


MouseHandler::MouseHandler()
    : _snot(new SelectNotifier), _vwr(nullptr), _mnow(nullptr), _mnxt(nullptr), _pnow(nullptr), _pnxt(nullptr) {}


MouseHandler::~MouseHandler()
{
    for ( const auto &p : _vwrs)
        static_cast<ModelViewer*>(p.second)->detach(this);
    _vwrs.clear();
    delete _snot;
}   // end dtor


void MouseHandler::addViewer( FMV *mv, bool isDefault)
{
    assert(mv);
    _vwrs[mv->getRenderWindow()] = mv;
    static_cast<ModelViewer*>(mv)->attach(this);
    if ( isDefault || !_vwr)
        _vwr = mv;
}   // end addViewer


int MouseHandler::registerHandler( GizmoHandler *gh)
{
    _gizmos.push_back(gh);
    return static_cast<int>(_gizmos.size()) - 1;
}   // end registerHandler


void MouseHandler::finishRegistration()
{
    for ( GizmoHandler *gh : _gizmos)
        gh->postRegister();
}   // end finishRegistration


void MouseHandler::refreshHandlers()
{
    for ( GizmoHandler *gh : _gizmos)
        gh->refresh();
}   // end refreshHandlers


bool MouseHandler::_handleEvent( const std::function<bool( GizmoHandler*)> &func) const
{
    // Ensure data is current for all gizmos (event disabled ones)
    const size_t N = _gizmos.size();
    for ( size_t i = 0; i < N; ++i)
    {
        GizmoHandler *gh = _gizmos.at(i);
        gh->_setViewer(_vwr);
        gh->_setProp(_pnow);
    }   // end for

    // Execute handler functions in order of handler addition.
    // If a handler swallows the event, don't execute on the remaining ones.
    bool swallowed = false;
    for ( size_t i = 0; i < N; ++i)
    {
        GizmoHandler *gh = _gizmos.at(i);
        if ( gh->isEnabled() && (swallowed = func(gh)))
            break;
    }   // end for

    return swallowed;
}   // end _handleEvent


FV* MouseHandler::_selectView( FV *nfv) const
{
    if ( !nfv)
    {
        const FV *sfv = _snot->selected();  // The currently selected FV

        if ( !nfv && sfv && (sfv->viewer() != _vwr))
        {
            // If the mouse viewer has only a single view attached, make this the selected one.
            if ( _vwr->attached().size() == 1)
                nfv = _vwr->attached().first();
            else
            {
                // Otherwise, set the selected view of the currently selected model as the one having _vwr as its viewer.
                for ( FV *fv : sfv->data()->fvs())
                {
                    if ( fv->viewer() == _vwr)
                    {
                        nfv = fv;
                        break;
                    }   // end if
                }   // end for
            }   // end else
        }   // end if
    }   // end if
    return nfv;
}   // end _selectView


void MouseHandler::mouseEnter( const QTools::VtkActorViewer *v)
{
    assert(_vwrs.count(v->getRenderWindow()) > 0);
    _vwr = _vwrs.at(v->getRenderWindow());
    _handleEvent( [](GizmoHandler *gh){ return gh->doEnterViewer();});
}   // end mouseEnter


bool MouseHandler::leftButtonDown()
{
    _setPointedAt();
    _snot->setSelected( _selectView(_mnxt), true);
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doLeftButtonDown();});
}   // end leftButtonDown


bool MouseHandler::rightButtonDown()
{
    _setPointedAt();
    _snot->setSelected( _selectView(_mnxt), true);
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doRightButtonDown();});
}   // end rightButtonDown


bool MouseHandler::middleButtonDown()
{
    _setPointedAt();
    _snot->setSelected( _selectView(_mnxt), true);
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMiddleButtonDown();});
}   // end middleButtonDown


bool MouseHandler::leftButtonUp()
{
    return _handleEvent( [](GizmoHandler *gh){ return gh->doLeftButtonUp();});
}   // end leftButtonUp


bool MouseHandler::leftDoubleClick()
{
    return _handleEvent( [](GizmoHandler *gh){ return gh->doLeftDoubleClick();});
}   // end leftDoubleClick


bool MouseHandler::middleButtonUp()
{
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMiddleButtonUp();});
}   // end middleButtonUp


bool MouseHandler::rightButtonUp()
{
    return _handleEvent( [](GizmoHandler *gh){ return gh->doRightButtonUp();});
}   // end rightButtonUp


bool MouseHandler::rightDoubleClick()
{
    return _handleEvent( [](GizmoHandler *gh){ return gh->doRightDoubleClick();});
}   // end rightDoubleClick


bool MouseHandler::leftDrag()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doLeftDrag();});
}   // end leftDrag


bool MouseHandler::middleDrag()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMiddleDrag();});
}   // end middleDrag


bool MouseHandler::rightDrag()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doRightDrag();});
}   // end rightDrag


bool MouseHandler::mouseMove()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMouseMove();});
}   // end mouseMove


bool MouseHandler::mouseWheelForward()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMouseWheelForward();});
}   // end mouseWheelForward


bool MouseHandler::mouseWheelBackward()
{
    _setPointedAt();
    _testMouseCursor();
    return _handleEvent( [](GizmoHandler *gh){ return gh->doMouseWheelBackward();});
}   // end mouseWheelBackward


void MouseHandler::_setPointedAt()
{
    _pnxt = _vwr->getPointedAt(_vwr->mouseCoords());   // The prop pointed at (may not be on current model)
    _mnxt = _vwr->attached().find(_pnxt); // The FaceView that the prop belongs to (if any)
    if ( _mnxt && _pnxt == _mnxt->actor())
        _pnxt = nullptr;
}   // end _setPointedAt


void MouseHandler::_testMouseCursor()
{
    if ( _pnow && (_pnow != _pnxt || _mnow != _mnxt))  // Leave previous prop? (which belongs to _mnow)
        _handleEvent( [](GizmoHandler *gh){ return gh->doLeaveProp();});

    if ( _mnow != _mnxt)
    {
        if ( _mnow)
            _handleEvent( [](GizmoHandler *gh){ return gh->doLeaveModel();});
        _mnow = _mnxt;
        if ( _mnow)    // Inform entering model
            _handleEvent( [](GizmoHandler *gh){ return gh->doEnterModel();});
    }   // end if

    if ( _pnow != _pnxt)
    {
        _pnow = _pnxt;
        if ( _pnow)      // Inform entering prop (which belongs to _mnow)
            _handleEvent( [](GizmoHandler *gh){ return gh->doEnterProp();});
    }   // end if
}   // end _testMouseCursor
