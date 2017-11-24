#include <ModelViewerQtInterface.h>
#include <QApplication>
#include <QDateTime>
using FaceTools::ModelViewerQtInterface;

ModelViewerQtInterface::ModelViewerQtInterface()
    : _camLockedCount(0), _lbDownTime(0), _rbDownTime(0)
{}   // end ctor


bool ModelViewerQtInterface::isCameraLocked() const
{
    return _camLockedCount > 0;
}   // end isCameraLocked


void ModelViewerQtInterface::setCameraLocked( bool v)
{
    if ( v)
        _camLockedCount++;
    else
        _camLockedCount--;
    if ( _camLockedCount < 0)
        _camLockedCount = 0;
}   // end setCameraLocked


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
    _mcoords = mc;
    emit onMiddleButtonDown();
}   // end signalOnMiddleButtonDown

void ModelViewerQtInterface::signalOnMiddleButtonUp( const QPoint& mc)
{
    _mcoords = mc;
    emit onMiddleButtonUp();
}   // end signalOnMiddleButtonUp

void ModelViewerQtInterface::signalOnRightButtonDown( const QPoint& mc)
{
    _mcoords = mc;
    _rbDownTime = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    emit onRightButtonDown();
}   // end signalOnRightButtonDown

void ModelViewerQtInterface::signalOnRightButtonUp( const QPoint& mc)
{
    _mcoords = mc;
    emit onRightButtonUp();

    const qint64 timeNow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    const bool rightUpFast = (timeNow - _rbDownTime) < QApplication::doubleClickInterval();
    // If right mouse released fast enough, then request the context menu.
    if ( rightUpFast)
        emit requestContextMenu( _mcoords);
}   // end signalOnRightButtonUp

void ModelViewerQtInterface::signalOnLeftButtonDown( const QPoint& mc)
{
    _mcoords = mc;
    const qint64 timeNow = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    const bool doubleClicked = (timeNow - _lbDownTime) < QApplication::doubleClickInterval();

    if ( doubleClicked)
    {
        _lbDownTime = 0;
        emit onLeftDoubleClick();
    }   // end if
    else
    {
        _lbDownTime = timeNow;
        emit onLeftButtonDown();
    }   // end if
}   // end signalOnLeftButton

void ModelViewerQtInterface::signalOnLeftButtonUp( const QPoint& mc)
{
    _mcoords = mc;
    emit onLeftButtonUp();
}   // end signalOnLeftButtonUp
