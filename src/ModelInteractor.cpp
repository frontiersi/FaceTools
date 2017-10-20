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
using FaceTools::FaceAction;
using FaceTools::FaceView;


// public
ModelInteractor::ModelInteractor( InteractiveModelViewer* viewer, FaceModel* fmodel, const QList<QAction*> *actions)
    : InteractionInterface(), _viewer(viewer),
     _fmodel(fmodel), _fview(NULL), _actions(actions),
     _isDrawingPath(false), _isMovingLandmark(false),
     _modelHoverOld(false), _lmHoverOld(""),
     _pickedLandmark(""), _interactive(false)
{
    _viewer->connectInterface(this);
    _fview = new FaceView( viewer, fmodel);

    // Make this interactor available to all of the passed in actions.
    if ( _actions)
    {
        foreach ( QAction* action, *_actions)
        {
            FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
            assert(faction);
            faction->addInteractor(this);
        }   // end foreach
    }   // end if
    else
        std::cerr << "[WARNING] FaceTools::ModelInteractor: NULL actions list provided!" << std::endl;
}   // end ctor


// public
ModelInteractor::~ModelInteractor()
{
    // Remove this interactor from all available actions.
    if ( _actions)
    {
        foreach ( QAction* action, *_actions)
        {
            FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
            faction->setInteractive( this, false);
            faction->removeInteractor( this);
        }   // end foreach
    }   // end if
    delete _fview;
}   // end dtor


// public
void ModelInteractor::setInteractive( bool enable)
{
    if ( enable != _interactive)
    {
        _interactive = enable;
        if ( _actions)
        {
            // Tell all actions that this interactor is switching interactivity.
            foreach ( QAction* action, *_actions)
            {
                FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
                faction->setInteractive( this, enable);
            }   // end foreach
        }   // end if
    }   // end if
}   // end setInteractive


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
        _viewer->setCursor( QCursor( Qt::BlankCursor));
        std::string lmk = _fview->isLandmarkPointedAt(p);
        if ( !lmk.empty() && _fview->areLandmarksShown() && _fview->getModel()->getObjectMeta()->getLandmarkMeta(lmk)->movable)
        {
            _pickedLandmark = lmk;
            _isMovingLandmark = true;
        }   // end else if
        else
            _isDrawingPath = true;
    }   // end if
}   // end leftButtonDown


// protected virtual
void ModelInteractor::leftButtonUp( const QPoint& p)
{
    if ( isDrawingPath())
        _fview->finishPath(p);
    _isMovingLandmark = false;
    _isDrawingPath = false;
    _pickedLandmark = "";
    _viewer->setCursor( QCursor(Qt::ArrowCursor));
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
    if ( isDrawingPath())
        _fview->drawPath(p);
    else if ( isMovingLandmark())
    {
        assert(!_pickedLandmark.empty());
        cv::Vec3f v;
        _fview->calcSurfacePosition(p, v);
        _fmodel->updateLandmark( _pickedLandmark, &v);
    }   // end else if
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
            _fmodel->highlightLandmark( _lmHoverOld, false);
            emit onExitingLandmark( _lmHoverOld, p);
        }   // end else if
        _lmHoverOld = lmHoverNow;
    }   // end if

    if ( _modelHoverOld && !modelHoverNow)
        emit onExitingModel(p);
    _modelHoverOld = modelHoverNow;
}   // end doMouseHover

