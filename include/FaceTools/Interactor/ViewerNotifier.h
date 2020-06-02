/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_VIEWER_NOTIFIER_H
#define FACE_TOOLS_VIEWER_NOTIFIER_H

#include <FaceTools/FaceTypes.h>
#include <QTools/VtkViewerInteractor.h>
#include <vtkRenderWindow.h>

namespace FaceTools { namespace Interactor {

class FaceTools_EXPORT ViewerNotifier : public QTools::VVI
{
public:
    ViewerNotifier();            // Attaches to ModelSelector::viewers().
    ~ViewerNotifier() override;  // Detaches from ModelSelector::viewers().

    // Return the viewer the mouse pointer was last over (never returns null).
    FMV* mouseViewer() const { return _vwr;}

    // Given a prop, return the FaceView iff the given prop is the main face actor
    // (and not some actor attached to a visualisation of the FaceView).
    Vis::FV* viewFromActor( const vtkProp3D*) const;

protected:
    // Called when the mouse has entered the parameter viewer (mouseViewer() returns parameter).
    virtual void enterViewer( FMV*) {}
    // Called when the mouse has left the parameter viewer.
    virtual void leaveViewer( FMV*) {}

/*
    virtual void cameraStart(){}
    virtual void cameraRotate(){}
    virtual void cameraDolly(){}
    virtual void cameraSpin(){}
    virtual void cameraPan(){}
    virtual void cameraMove(){} // Generic non-specific ongoing movement
    virtual void cameraStop(){} // After camera movement stopped.

    virtual void actorStart( const vtkProp3D*){}
    virtual void actorRotate( const vtkProp3D*){}
    virtual void actorDolly( const vtkProp3D*){}
    virtual void actorSpin( const vtkProp3D*){}
    virtual void actorPan( const vtkProp3D*){}
    virtual void actorMove( const vtkProp3D*){} // Generic non-specific ongoing movement
    virtual void actorStop( const vtkProp3D*){} // After actor movement stopped.
*/

private:
    void mouseEnter( const QTools::VtkActorViewer*) override;
    void mouseLeave( const QTools::VtkActorViewer*) override;
    std::unordered_map<const vtkRenderWindow*, FMV*> _vwrs;
    FMV* _vwr;

    ViewerNotifier( const ViewerNotifier&) = delete;
    void operator=( const ViewerNotifier&) = delete;
};  // end class

}}   // end namespace

#endif
