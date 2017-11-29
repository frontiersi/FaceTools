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

#include <FaceControl.h>
#include <LegendRange.h>
#include <BoundaryView.h>
#include <OutlinesView.h>
#include <LandmarkGroupView.h>
#include <VisualisationAction.h>
#include <InteractiveModelViewer.h>
#include <vtkProperty.h>
using FaceTools::InteractionInterface;
using FaceTools::ModelOptions;
using FaceTools::VisualisationAction;
using FaceTools::LandmarkGroupView;
using FaceTools::OutlinesView;
using FaceTools::BoundaryView;
using FaceTools::FaceControl;
using FaceTools::LegendRange;
using FaceTools::FaceAction;
using FaceTools::FaceModel;
using FaceTools::FaceView;


// public
FaceControl::FaceControl( FaceModel* fmodel)
    : InteractionInterface(), _viewer(NULL), _fmodel(fmodel),
     _fview( new FaceView( fmodel->getObjectMeta()->getObject())),
     _oview( new OutlinesView( fmodel->getObjectMeta()->getObject())),
     _bview( new BoundaryView( fmodel->getObjectMeta())),
     _lview( new LandmarkGroupView( fmodel->getObjectMeta())),
     _legend( new LegendRange),
     _curvis(NULL),
     _modelHoverOld(false), _lmHoverOld(""), _controlled(false)
{
    connect( fmodel, &FaceModel::metaUpdated, this, &FaceControl::metaUpdated);
    connect( fmodel, &FaceModel::meshUpdated, this, &FaceControl::doMeshUpdated);
}   // end ctor


// public
FaceControl::~FaceControl()
{
    foreach ( QAction* action, _actions) // Hide this interactor from all passed in actions.
    {
        FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
        faction->setControlled( this, false);
        removeController(action);
    }   // end foreach
    delete _legend;
    delete _lview;
    delete _bview;
    delete _oview;
    delete _fview;
    if ( _viewer)
        _viewer->updateRender();
}   // end dtor


// public
void FaceControl::addController( QAction* action)
{
    if (_actions.count(action) > 0) // Don't add the same action more than once
        return;
    _actions.insert(action);
    FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
    assert(faction);
    faction->addController( this);
}   // end addController


// public
void FaceControl::removeController( QAction* action)
{
    _actions.erase(action);
    FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
    assert(faction);
    faction->removeController( this);
}   // end removeController


// public
void FaceControl::setControlled( bool enable)
{
    if ( enable != _controlled)
    {
        _controlled = enable;
        // Tell all actions that this interactor is switching interactivity.
        foreach ( QAction* action, _actions)
        {
            FaceAction* faction = qobject_cast<FaceAction*>(action->parent());
            faction->setControlled( this, enable);
        }   // end foreach
    }   // end if
}   // end setControlled


// public
bool FaceControl::isControlled() const { return _controlled;}


// public
void FaceControl::setViewer( FaceTools::InteractiveModelViewer* viewer)
{
    _fview->setVisible( _fview->isVisible() && viewer, viewer);
    _oview->setVisible( _oview->isVisible() && viewer, viewer);
    _bview->setVisible( _bview->isVisible() && viewer, viewer);
    _lview->setVisible( _lview->isVisible() && viewer, viewer);
    if ( _viewer)
        _viewer->updateRender();
    _viewer = viewer;
    if ( _viewer)
        _viewer->updateRender();
}   // end setViewer


// private
void FaceControl::setScalarVisualisation()
{
    std::string vname;
    float fmin, fmax;
    if ( _curvis->allowScalarVisualisation( fmin, fmax))
    {
        vname = _curvis->getDisplayName().toStdString();
        if ( !_legend->gotMapping(vname))
            _legend->setBounds( vname, fmin, fmax);
    }   // end if
    _legend->setActor( _fview->getSurfaceActor());
    _legend->setVisible( vname, _viewer);
}   // end setScalarVisualisation


// public
void FaceControl::setVisualisation( VisualisationAction* vaction)
{
    _curvis = vaction;
    assert(vaction);
    assert(vaction->isAvailable(this));

    _curvis->mapActor(this);
    _fview->setTexture( vaction->useTexture());
    if ( _viewer)
        _viewer->enableFloodLights( vaction->useTexture());

    setScalarVisualisation();
    _fview->setVisible( true, _viewer);

    emit viewUpdated();
    if ( _viewer)
        _viewer->updateRender();
}   // end setVisualisation


// public
void FaceControl::setOptions( const ModelOptions& vo)
{
    _opts = vo;
    _fview->setOptions( _opts);

    // Set legend visualisation options
    if ( _curvis && _legend->gotMapping( _curvis->getDisplayName().toStdString()))
    {
        const QColor& mcol0 = _opts.model.surfaceColourMin;
        const QColor& mcol1 = _opts.model.surfaceColourMid;
        const QColor& mcol2 = _opts.model.surfaceColourMax;
        _legend->setColours( mcol0, mcol1, mcol2);
        _legend->setNumColours( _opts.model.numSurfaceColours);
        _legend->setScalarMappingMinMax( _opts.model.minVisibleScalar, _opts.model.maxVisibleScalar);
    }   // end if

    _lview->setOptions( _opts);
    _bview->setOptions( _opts.boundary);
}   // end setOptions


// public
bool FaceControl::belongs( const vtkProp* prop) const
{
    bool isprop = false;
    if ( prop)
    {
        isprop = _fview->getActor() == prop || _lview->isLandmark(prop);
        if ( !isprop)   // Check the temp members
            isprop = isTempPropPointedAt( prop);
    }   // end if
    return isprop;
}   // end belongs


// public
bool FaceControl::isPointedAt() const
{
    return belongs( _viewer->getPointedAt());
}   // end isPointedAt


// public
void FaceControl::addTempMemberProp( const vtkProp* p)
{
    _tmpProps.insert(p);
}   // end addTempMemberProp


// public
void FaceControl::removeTempMemberProp( const vtkProp* p)
{
    _tmpProps.erase(p);
}   // end removeTempMemberProp


// public
std::string FaceControl::isLandmarkPointedAt( const QPoint* p) const
{
    return _lview->pointedAt( p ? *p : getMouseCoords());
}   // end isLandmarkPointedAt


// public
bool FaceControl::isTempPropPointedAt( const vtkProp* prop) const
{
    if (!prop)
        prop = _viewer->getPointedAt();
    if ( !prop)
        return false;

    bool istmp = false;
    foreach ( const vtkProp* tp, _tmpProps)
    {
        if ( tp == prop)
        {
            istmp = true;
            break;
        }   // end if
    }   // end foreach
    return istmp;
}   // end isTempPropPointedAt


// protected virtual
void FaceControl::leftButtonDown( const QPoint& p)
{
    if ( isPointedAt())
        emit onLeftButtonDown(p);
}   // end leftButtonDown


void FaceControl::leftButtonUp( const QPoint& p)
{
    emit onLeftButtonUp(p);
}   // end leftButtonUp


void FaceControl::leftDoubleClick( const QPoint& p)
{
    if ( isPointedAt())
        emit onLeftDoubleClick(p);
}   // end leftDoubleClick


void FaceControl::leftDrag( const QPoint& p)
{
    if ( isPointedAt() || isTempPropPointedAt())
        emit onLeftDrag(p);
}   // end leftDrag


void FaceControl::mouseMove( const QPoint& p)
{
    const bool modelHoverNow = isPointedAt();
    if ( modelHoverNow)
        emit onMouseMove(p);

    if ( _modelHoverOld && !modelHoverNow)
        emit onExitingModel(p);

    if ( modelHoverNow && areLandmarksShown()) // Check landmark entry/exit
    {
        const std::string lmHoverNow = isLandmarkPointedAt(&p);

        if ( _lmHoverOld != lmHoverNow)
        {
            if ( !_lmHoverOld.empty())  // Transitioned from a different landmark
                emit onExitingLandmark( _lmHoverOld, p);

            if ( !lmHoverNow.empty())
                emit onEnteringLandmark( lmHoverNow, p);
        }   // end if

        _lmHoverOld = lmHoverNow;
    }   // end if

    if ( !_modelHoverOld && modelHoverNow)
        emit onEnteringModel(p);

    _modelHoverOld = modelHoverNow;
}   // end mouseMove


// public
bool FaceControl::isSelected() const { return _oview->isVisible();}
bool FaceControl::isBoundaryShown() const { return _bview->isVisible();}
bool FaceControl::areLandmarksShown() const { return _lview->isVisible();}


// public
void FaceControl::showSelected( bool enable)
{
    // Move these details into FaceView at some point.
    _oview->setVisible(enable, _viewer);
    //const QColor& scol = _opts.model.surfaceColourMid;
    //_fview->getSurfaceActor()->GetProperty()->SetColor( scol.redF(), scol.greenF(), scol.blueF());
    //double alpha = _opts.model.opacity;
    //if (!enable)
    //    alpha /= 2.0;
    //_fview->getSurfaceActor()->GetProperty()->SetOpacity( alpha);
    if ( _viewer)
        _viewer->updateRender();
}   // end showSelected


// public
void FaceControl::showBoundary( bool enable)
{
    const bool changed = enable != _bview->isVisible();
    _bview->setVisible(enable, _viewer);
    if ( changed)
        emit viewUpdated();
    if ( _viewer)
        _viewer->updateRender();
}   // end showBoundary


// public
void FaceControl::showLandmarks( bool enable)
{
    const bool changed = enable != _lview->isVisible();
    _lview->setVisible(enable, _viewer);
    if ( changed)
        emit viewUpdated();
    if ( _viewer)
        _viewer->updateRender();
}   // end showLandmarks


// public
void FaceControl::showLandmark( bool enable, const std::string& lm)
{
    const bool changed = enable != _lview->isLandmarkVisible( lm);
    _lview->showLandmark(enable, lm);
    if ( changed)
        emit viewUpdated();
    if ( _viewer)
        _viewer->updateRender();
}   // end showLandmark


// public
void FaceControl::highlightLandmark( bool enable, const std::string& lm)
{
    _lview->highlightLandmark(enable, lm);
    if ( _viewer)
        _viewer->updateRender();
}   // end highlightLandmark


// public
void FaceControl::updateLandmark( const std::string& lm, const cv::Vec3f* v, bool updateModel)
{
    if ( updateModel)
        _fmodel->updateLandmark(lm, v);
    _lview->updateLandmark(lm, v);
    if ( _viewer)
        _viewer->updateRender();
}   // end updateLandmark


// public
void FaceControl::updateLandmark( const FaceTools::Landmarks::Landmark& lmk)
{
    _fmodel->updateLandmark(lmk);
    _lview->updateLandmark( lmk.name, &lmk.pos);
    if ( _viewer)
        _viewer->updateRender();
}   // end updateLandmark


// public
void FaceControl::updateMesh( const RFeatures::ObjModel::Ptr model)
{
    _fmodel->updateMesh(model);
}   // end updateMesh


// public
void FaceControl::resetVisualisation()
{
    _fview->reset(_fmodel->getObjectMeta()->getObject());
    delete _legend;
    _legend = new LegendRange;

    const bool showingOutline = isSelected();
    _oview->reset(_fmodel->getObjectMeta()->getObject());

    const bool showingBoundary = _bview->isVisible();
    _bview->reset();

    std::vector<std::string> visibleLandmarks;
    _lview->getVisibleLandmarks( visibleLandmarks);
    _lview->reset();

    // Restore the views
    showSelected( showingOutline);
    _bview->setVisible( showingBoundary, _viewer);
    _lview->setVisible( false, _viewer);
    foreach ( const std::string& lm, visibleLandmarks)
        _lview->showLandmark( true, lm);

    setVisualisation( _curvis);
}   // end resetVisualisation


// private slot
void FaceControl::doMeshUpdated()
{
    resetVisualisation();
    emit meshUpdated();
}   // end doMeshUpdated


