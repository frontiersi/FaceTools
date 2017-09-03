#include <ModelViewerQtInterface.h>
#include <QApplication>
#include <QDateTime>
using FaceTools::ModelViewerQtInterface;

ModelViewerQtInterface::ModelViewerQtInterface()
    : _cameraLocked(false), _lbDownTime(0)
{}   // end ctor

void ModelViewerQtInterface::signalOnMouseEnter( const QPoint& mc)
{
    _mcoords = mc;
    emit onMouseEnter();
}   // end signalOnMouseEnter

void ModelViewerQtInterface::signalOnMouseLeave( const QPoint& mc)
{
    _mcoords = mc;
    emit onMouseLeave();
}   // end signalOnMouseLeave

void ModelViewerQtInterface::signalOnMouseMove( const QPoint& mc)
{
    _mcoords = mc;
    emit onMouseMove();
}   // end signalOnMouseMove

void ModelViewerQtInterface::signalOnMouseWheelForward( const QPoint& mc)
{
    _mcoords = mc;
    emit onMouseWheelForward();
}   // end signalOnMouseWheelForward

void ModelViewerQtInterface::signalOnMouseWheelBackward( const QPoint& mc)
{
    _mcoords = mc;
    emit onMouseWheelBackward();
}   // end signalOnMouseWheelBackward

void ModelViewerQtInterface::signalOnMiddleButtonDown( const QPoint& mc)
{
    if ( _cameraLocked)
        emit unlockedCamera();
    _cameraLocked = false;
    _mcoords = mc;
    emit onMiddleButtonDown();
}   // end signalOnMiddleButtonDown

void ModelViewerQtInterface::signalOnMiddleButtonUp( const QPoint& mc)
{
    if ( !_cameraLocked)
        emit lockedCamera();
    _cameraLocked = true;
    _mcoords = mc;
    emit onMiddleButtonUp();
}   // end signalOnMiddleButtonUp

void ModelViewerQtInterface::signalOnRightButtonDown( const QPoint& mc)
{
    _mcoords = mc;
    emit onRightButtonDown();
}   // end signalOnRightButtonDown

void ModelViewerQtInterface::signalOnRightButtonUp( const QPoint& mc)
{
    _mcoords = mc;
    emit onRightButtonUp();
}   // end signalOnRightButtonUp

void ModelViewerQtInterface::signalOnLeftButtonDown( const QPoint& mc)
{
    _mcoords = mc;
    // Lock the camera from moving if not a double left click.
    // Camera can be moved only when double clicking and dragging.
    const qint64 timeNow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    const bool doubleClicked = (timeNow - _lbDownTime) < QApplication::doubleClickInterval();

    if ( _cameraLocked && doubleClicked)
        emit unlockedCamera();

    _cameraLocked = !doubleClicked;
    if ( _cameraLocked)
    {
        _lbDownTime = timeNow;
        emit onLeftButtonDown();
    }   // end if
    else
    {
        _lbDownTime = 0;
        emit onLeftDoubleClick();
    }   // end if
}   // end signalOnLeftButton

void ModelViewerQtInterface::signalOnLeftButtonUp( const QPoint& mc)
{
    if ( !_cameraLocked)
        emit lockedCamera();
    _cameraLocked = true;
    _mcoords = mc;
    emit onLeftButtonUp();
}   // end signalOnLeftButtonUp
