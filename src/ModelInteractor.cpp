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

#include <ModelInteractor.h>
using FaceTools::ModelInteractor;
using FaceTools::InteractiveModelViewer;
using FaceTools::InteractionInterface;
using FaceTools::FaceView;


// public
ModelInteractor::ModelInteractor( InteractiveModelViewer* viewer, FaceModel* fmodel, QActionGroup* agroup)
    : InteractionInterface(), _viewer(viewer),
     _fmodel(fmodel), _fview(NULL),
     _isDrawingPath(false), _isMovingLandmark(false),
     _modelHoverOld(false), _lmHoverOld(""),
     _pickedLandmark(""), _actionable(false)
{
    _viewer->connectInterface(this);

    // Create the view and connect up listener to the boundary view observer
    _fview = new FaceView( viewer, fmodel, &_bobserver);
    connect( &_bobserver, &FaceTools::BoundaryViewEventObserver::updatedBoundary, _fmodel, &FaceTools::FaceModel::updateBoundary);

    // Make this interactor available to all of the passed in actions.
    foreach ( QAction* action, agroup->actions())
    {
        FaceActionInterface* faction = qobject_cast<FaceActionInterface*>(action->parent());
        assert(faction);
        faction->connectInteractor(this);
    }   // end foreach
}   // end ctor


// public
ModelInteractor::~ModelInteractor()
{
    delete _fview;
}   // end dtor


// public
void ModelInteractor::setActionable( bool enable)
{
    if ( enable != _actionable)
    {
        _actionable = enable;
        emit enableActionable( enable);
    }   // end if
}   // end setActionable


// public
bool ModelInteractor::calcSurfacePosition( cv::Vec3f& v) const
{
    const QPoint& p = getMouseCoords();
    return getView()->calcSurfacePosition( p, v);
}   // end calcSurfacePosition


// protected virtual
void ModelInteractor::middleDrag( const QPoint& p)
{
    _viewer->setCursor( QCursor(Qt::DragMoveCursor));
}   // end middleDrag


// protected virtual
void ModelInteractor::middleButtonUp( const QPoint& p)
{
    _viewer->setCursor( QCursor(Qt::ArrowCursor));
}   // end middleButtonUp


// protected virtual
void ModelInteractor::leftButtonDown( const QPoint& p)
{
    if ( _fview && _fview->isPointedAt(p))
    {
        _viewer->setCursor( QCursor(Qt::CrossCursor));
        _pickedLandmark = _fview->isLandmarkPointedAt(p);
        if ( _pickedLandmark.empty())
        {
            _isDrawingPath = true;
            _fview->drawPath(p);
        }   // end if
        else if ( _fview->getModel()->getObjectMeta()->getLandmarkMeta(_pickedLandmark)->movable)
        {
            _isMovingLandmark = true;
            cv::Vec3f v;
            _fview->calcSurfacePosition(p, v);
            _fmodel->updateLandmark( _pickedLandmark, &v);
        }   // end else if
    }   // end if
}   // end leftButtonDown


// protected virtual
void ModelInteractor::leftButtonUp( const QPoint& p)
{
    _viewer->setCursor( QCursor(Qt::ArrowCursor));
    if ( !_pickedLandmark.empty())
    {
        if ( _fview->getModel()->getObjectMeta()->getLandmarkMeta(_pickedLandmark)->movable)
        {
            cv::Vec3f v;
            _fview->calcSurfacePosition(p, v);
            _fmodel->updateLandmark( _pickedLandmark, &v);
            _isMovingLandmark = false;
        }   // end if
        _pickedLandmark = "";
    }   // end if
    else if ( isCameraLocked())
    {
        _fview->finishPath(p);
        _isDrawingPath = false;
    }   // end else

    doMouseHover(p);
}   // end leftButtonUp


// protected virtual
void ModelInteractor::leftDoubleClick( const QPoint& p)
{
    if ( _fview && _fview->isPointedAt(p))
        _viewer->setCursor( QCursor(Qt::DragMoveCursor));
}   // end leftDoubleClick


// protected virtual
void ModelInteractor::leftDrag( const QPoint& p)
{
    if ( _fview && _fview->isPointedAt(p))
    {
        if ( !_pickedLandmark.empty() && _fview->getModel()->getObjectMeta()->getLandmarkMeta(_pickedLandmark)->movable)
        {
            _isMovingLandmark = true;
            cv::Vec3f v;
            _fview->calcSurfacePosition(p, v);
            _fmodel->updateLandmark( _pickedLandmark, &v);
        }   // end if
        else if ( isCameraLocked())
        {
            _isDrawingPath = true;
            _fview->drawPath(p);
        }   // end else if
    }   // end if
}   // end leftDrag


// protected virtual
void ModelInteractor::mouseMove( const QPoint& p)
{
    doMouseHover(p);
}   // end mouseMove


// private
void ModelInteractor::doMouseHover( const QPoint& p)
{
    const bool modelHoverNow = _fview->isPointedAt(p);
    if ( !_modelHoverOld && modelHoverNow)
        emit onEnteringModel(p);

    if ( modelHoverNow)
    {
        const std::string lmHoverNow = _fview->isLandmarkPointedAt(p);
        if ( _lmHoverOld.empty() && !lmHoverNow.empty())
        {
            _fmodel->highlightLandmark( lmHoverNow, true);
            emit onEnteringLandmark( lmHoverNow, p);
        }   // end if
        else if ( !_lmHoverOld.empty() && lmHoverNow.empty())
        {
            _fmodel->highlightLandmark( lmHoverNow, false);
            emit onExitingLandmark( lmHoverNow, p);
        }   // end else if
        _lmHoverOld = lmHoverNow;
    }   // end if

    if ( _modelHoverOld && !modelHoverNow)
        emit onExitingModel(p);
    _modelHoverOld = modelHoverNow;
}   // end doMouseHover

