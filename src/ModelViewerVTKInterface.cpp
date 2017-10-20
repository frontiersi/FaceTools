#include <ModelViewerVTKInterface.h>
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRendererCollection.h>
#include <vtkObjectFactory.h>   // vtkStandardNewMacro
using FaceTools::ModelViewerVTKInterface;

vtkStandardNewMacro( ModelViewerVTKInterface)


ModelViewerVTKInterface::ModelViewerVTKInterface()
    : vtkInteractorStyleTrackballCamera()
{ }   // end ctor


void ModelViewerVTKInterface::OnLeftButtonDown()
{
    _qtinterface.signalOnLeftButtonDown( getMouseCoords());
    vtkInteractorStyleTrackballCamera::OnLeftButtonDown();
}   // end OnLeftButtonDown


void ModelViewerVTKInterface::OnLeftButtonUp()
{
    vtkInteractorStyleTrackballCamera::OnLeftButtonUp();
    _qtinterface.signalOnLeftButtonUp( getMouseCoords());
}   // end OnLeftButtonUp


void ModelViewerVTKInterface::OnRightButtonDown()
{
    //vtkInteractorStyleTrackballCamera::OnRightButtonDown();
    _qtinterface.signalOnRightButtonDown( getMouseCoords());
}   // end OnRightButtonDown


void ModelViewerVTKInterface::OnRightButtonUp()
{
    //vtkInteractorStyleTrackballCamera::OnRightButtonUp();
    _qtinterface.signalOnRightButtonUp( getMouseCoords());
}   // end OnRightButtonUp


void ModelViewerVTKInterface::OnMiddleButtonDown()
{
    vtkInteractorStyleTrackballCamera::OnMiddleButtonDown();
    _qtinterface.signalOnMiddleButtonDown( getMouseCoords());
}   // end OnMiddleButtonDown

void ModelViewerVTKInterface::OnMiddleButtonUp()
{
    vtkInteractorStyleTrackballCamera::OnMiddleButtonUp();
    _qtinterface.signalOnMiddleButtonUp( getMouseCoords());
}   // end OnMiddleButtonUp

void ModelViewerVTKInterface::OnMouseWheelForward()
{
    //if ( !_qtinterface.isCameraLocked())
    vtkInteractorStyleTrackballCamera::OnMouseWheelForward();   // Do zoom
    _qtinterface.signalOnMouseWheelForward( getMouseCoords());
}   // end OnMouseWheelForward

void ModelViewerVTKInterface::OnMouseWheelBackward()
{
    //if ( !_qtinterface.isCameraLocked())
    vtkInteractorStyleTrackballCamera::OnMouseWheelBackward();
    _qtinterface.signalOnMouseWheelBackward( getMouseCoords());
}   // end OnMouseWheelBackward


void ModelViewerVTKInterface::OnMouseMove()
{
    if ( !_qtinterface.isCameraLocked())
        vtkInteractorStyleTrackballCamera::OnMouseMove();
    _qtinterface.signalOnMouseMove( getMouseCoords());
}   // end OnMouseMove


void ModelViewerVTKInterface::OnEnter()
{
    vtkInteractorStyleTrackballCamera::OnEnter();
    _qtinterface.signalOnMouseEnter( getMouseCoords());
}   // end OnEnter


void ModelViewerVTKInterface::OnLeave()
{
    vtkInteractorStyleTrackballCamera::OnLeave();
    _qtinterface.signalOnMouseLeave( getMouseCoords());
}   // end OnLeave


// private
// VTK 2D origin is at bottom left of render window so need to set to top left.
QPoint ModelViewerVTKInterface::getMouseCoords()
{
    vtkSmartPointer<vtkRenderer> ren = this->GetInteractor()->GetRenderWindow()->GetRenderers()->GetFirstRenderer();
    return QPoint( this->GetInteractor()->GetEventPosition()[0],
                      ren->GetSize()[1] - this->GetInteractor()->GetEventPosition()[1] - 1);
}   // end getMouseCoords
