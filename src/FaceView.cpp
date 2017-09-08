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

#include <FaceView.h>
#include <FaceTools.h>
#include <vtkProperty.h>
#include <cassert>
using FaceTools::FaceModel;
using FaceTools::ModelViewer;
using FaceTools::FaceView;
using FaceTools::VisualisationAction;
using FaceTools::BoundaryViewEventObserver;


// public
FaceView::FaceView( ModelViewer* viewer, const FaceModel* fmodel, BoundaryViewEventObserver* bobserver)
    : _viewer(viewer), _fmodel(fmodel), _curvis(NULL), _inview(false),
      _bview( viewer, fmodel->getObjectMeta(), bobserver),
      _lview( viewer, fmodel->getObjectMeta()),
      _pathDrawer( viewer)
{
    // This view updates itself in response to changes on the model, not the other way around.
    // Designed like this because there can be multiple views for a model, all providing means
    // for the user to change the model's data - and we want changes to the model's data to
    // be propagated back to all of the views (not just the one that it was changed through).
    connect( _fmodel, &FaceModel::onLandmarkUpdated, &_lview, &FaceTools::LandmarkGroupView::updateLandmark);
    connect( _fmodel, &FaceModel::onLandmarkSelected, &_lview, &FaceTools::LandmarkGroupView::selectLandmark);
    connect( _fmodel, &FaceModel::onMeshUpdated, this, &FaceView::forceRevisualise);
    connect( _fmodel, &FaceModel::onFaceDetected, this, &FaceView::showFaceDetection);
    connect( _fmodel, &FaceModel::onTransformed, this, &FaceView::setCameraToOrigin);
}   // end ctor


// public
FaceView::~FaceView()
{
    _lview.showAll(false);
    _bview.show(false);
    if ( isModelShown())
        showModel(false);
}   // end dtor


// public
bool FaceView::isModelShown() const
{
    return _inview;
}   // end isModelShown


// public
bool FaceView::isBoundaryShown() const
{
    return _bview.isShown();
}   // end isBoundaryShown


// public
bool FaceView::canAdjustBoundary() const
{
    return _bview.canAdjust();
}   // end canAdjustBoundary


// public
bool FaceView::areLandmarksShown() const
{
    return _lview.isShown();
}   // end areLandmarksShown


// public
bool FaceView::canVisualise( VisualisationAction* visint) const
{
    return visint->isAvailable(_fmodel);
}   // end canVisualise


// public
bool FaceView::isPointedAt( const QPoint &p) const
{
    const vtkProp* prop = _viewer->getPointedAt(p);
    return ( prop == getActor()) || !_lview.pointedAt(p).empty();
}   // end isPointedAt


// public
bool FaceView::calcSurfacePosition( const QPoint& p, cv::Vec3f& v) const
{
    return _viewer->calcSurfacePosition( getActor(), p, v);
}   // end calcSurfacePosition


// public
std::string FaceView::isLandmarkPointedAt( const QPoint &p) const
{
    return _lview.pointedAt(p);
}   // end isLandmarkPointedAt


// public
RFeatures::CameraParams FaceView::getCamera() const
{
    return _viewer->getCamera();
}   // end getCamera


// public
const vtkActor* FaceView::getActor() const
{
    assert( _curvis);
    return _allvis.at(_curvis);
}   // end getActor


// public slot
void FaceView::visualise( VisualisationAction* visint)
{
    assert( visint->isAvailable(_fmodel));

    // Remove current visualisation if shown
    if ( isModelShown())
        _viewer->remove( getActor());
    _curvis = visint;

    if ( _allvis.count(_curvis) == 0) // Generate the model if necessary
        _allvis[_curvis] = _curvis->makeActor(_fmodel);

    _pathDrawer.setModel( _allvis.at(_curvis));
    showModel(true);
}   // end visualise


// public slot
void FaceView::showModel( bool enable)
{
    assert( _curvis != NULL);
    _inview = enable;

    // If showing the model, ensure that the viewer is set up correctly
    _viewer->enableFloodLights( _inview && _curvis->useFloodLights());

    float minv, maxv;
    const bool showLegend = _curvis->allowScalarVisualisation( minv, maxv);
    _viewer->showLegend( _inview && showLegend);

    vtkSmartPointer<vtkActor> actor = _allvis[_curvis];
    if ( !_inview)
        _viewer->remove( actor);
    else
    {
        if ( !showLegend)
            _viewer->add( actor);
        else
        {
            // minv and maxv are the allowed ranges from the visualisation.
            // Get the desired min and max from the model options.
            const float userminv = std::max( minv, std::min( _visopts.model.minVisibleScalar, maxv));
            const float usermaxv = std::min( maxv, std::max( _visopts.model.maxVisibleScalar, minv));
            _viewer->add( actor, _curvis->getDisplayName().toStdString(), userminv, usermaxv);
        }   // end else
    }   // end else

    applyVisualisationOptions(_visopts);
}   // end showModel


// public slot
void FaceView::applyVisualisationOptions( const FaceTools::VisualisationOptions& visopts)
{
    assert( _curvis);
    _visopts = visopts;

    // Modify the actor according to the visualisation options
    vtkSmartPointer<vtkActor> actor = _allvis[_curvis];
    if ( _curvis->allowSetBackfaceCulling())
        actor->GetProperty()->SetBackfaceCulling( _visopts.model.backfaceCulling);
    if ( _curvis->allowSetVertexSize())
        actor->GetProperty()->SetPointSize( _visopts.model.vertexSize);
    if ( _curvis->allowSetWireframeLineWidth())
        actor->GetProperty()->SetLineWidth( _visopts.model.lineWidth);
    if ( _curvis->allowSetColour())
    {
        const QColor& mscol = _visopts.model.surfaceColourFlat;
        actor->GetProperty()->SetColor( mscol.redF(), mscol.greenF(), mscol.blueF());
        actor->GetProperty()->SetOpacity( mscol.alphaF());
    }   // end if

    const QColor& mcol0 = _visopts.model.surfaceColourMin;
    const QColor& mcol1 = _visopts.model.surfaceColourMax;
    _viewer->setLegendColours( mcol0, mcol1, _visopts.model.numSurfaceColours);

    _lview.setVisualisationOptions( _visopts.landmarks);
    _bview.setVisualisationOptions( _visopts.boundary);
    _viewer->updateRender();
}   // end applyVisualisationOptions


// public slot
void FaceView::showBoundary( bool enable)
{
    _bview.show(enable);
    applyVisualisationOptions(_visopts);
}   // end showBoundary


// public slot
void FaceView::allowBoundaryAdjustment( bool enable)
{
    _bview.allowAdjustment(enable);
}   // end allowBoundaryAdjustment


// public slot
void FaceView::showLandmarks( bool enable)
{
    _lview.showAll(enable);
    applyVisualisationOptions(_visopts);
}   // end showLandmarks


// public slot
void FaceView::showLandmark( const std::string& lm, bool enable)
{
    _lview.showLandmark( lm, enable);
    applyVisualisationOptions(_visopts);
}   // end showLandmark


// public slot
void FaceView::highlightLandmark( const std::string& lm, bool enable)
{
    _lview.highlightLandmark( lm, enable);
    applyVisualisationOptions(_visopts);
}   // end highlightLandmark


// protected
void FaceView::drawPath( const QPoint& p)
{
    _pathDrawer.doDrawingPath(p);
}   // end drawPath


// protected
void FaceView::finishPath( const QPoint& p)
{
    _pathDrawer.doFinishedPath(p);
}   // end finishPath


// private slot
void FaceView::forceRevisualise()
{
    // Remove the existing visualisation from the viewer if it exists
    if ( _curvis && _allvis.count(_curvis) > 0)
        _viewer->remove( getActor());
    // Clear all generated visualisations forcing them to be renewed on next call to visualise
    _allvis.clear();
    if ( _curvis)
        visualise(_curvis);
}   // end forceRevisualise


// private slot
void FaceView::showFaceDetection()
{
    _lview.erase();    // Erase existing!
    _lview.reset();    // Reset from ObjMetaData
    _bview.reset( getActor()); // Reset from ObjMetaData
    orientCameraToFace();
}   // end showFaceDetection


// public slot
void FaceView::orientCameraToFace()
{
    const FaceTools::ObjMetaData::Ptr objmeta = _fmodel->getObjectMeta();
    const cv::Vec3f focus = FaceTools::calcFaceCentre( objmeta);
    cv::Vec3f nvec, uvec;
    objmeta->getOrientation( nvec, uvec);
    _viewer->setCamera( focus, nvec, uvec);
    applyVisualisationOptions(_visopts);
}   // end orientCameraToFace


// public slot
bool FaceView::saveSnapshot() const
{
    return _viewer->saveSnapshot();
}   // end saveSnapshot


// public slot
bool FaceView::setCameraFocus( const QPoint& p)
{
    cv::Vec3f newFocus;
    if ( !calcSurfacePosition( p, newFocus))
        return false;

    RFeatures::CameraParams cam = _viewer->getCamera();
    cam.focus = newFocus;
    _viewer->setCamera( cam);
    _viewer->updateRender();
    return true;
}   // end setCameraFocus


// public slot
void FaceView::setCameraToOrigin()
{
    _viewer->resetDefaultCamera();
    _viewer->updateRender();
}   // end setCameraToOrigin
