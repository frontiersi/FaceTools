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

#include <FaceModelWidget.h>
#include <ui_FaceModelWidget.h>
#include <FaceModelView.h>
using FaceTools::FaceModelWidget;

using FaceTools::VisualisationInterface;
using FaceTools::SurfacePathDrawer;


const FM* getCurrentlySelectedModel( const FaceModelWidget* mvh)
{
    const int modelID = mvh->getCurrentlySelectedComboBoxModelID();
    if ( modelID <= 0)
        return NULL;
    const FM* fm = FMM::getModel( modelID);
    assert(fm);
    return fm;
}   // end getCurrentlySelectedModel



QAction* copyAction( const QAction* a)
{
    QAction* na = new QAction( a->text(), a->parent());
    na->setCheckable( a->isCheckable());
    na->setIcon(a->icon());
    return na;
}   // end copyAction


void setButton( QToolButton* tb, QAction* action)
{
    tb->addAction( action);
    tb->setIcon( action->icon());
    tb->setIconSize( QSize(20,20));
    tb->setMinimumSize( QSize(26,26));
    tb->setStyleSheet( "QToolButton::menu-indicator { image: none; }"); // Remove the menu-indicator
    QObject::connect( tb, SIGNAL(clicked(bool)), action, SIGNAL(triggered(bool)));
}   // end setButton


// private
void FaceModelWidget::addActionsToVisualisationToolBar( const QActionGroup& viewGroup)
{
    _visualisationToolBar->setOrientation( Qt::Vertical);
    ui->visualisationToolbarFrame->layout()->addWidget( _visualisationToolBar);

    int shortcutNum = 1;
    // Copy out the ViewGroup actions into this modules own actions
    foreach ( const QAction* action, viewGroup.actions())
    {
        QAction* newAction = copyAction( action);    // Copy action (with parent being VisualisationInterface)
        assert( newAction->parent() != NULL);

        if ( shortcutNum < 10)  // Set a shortcut if we've not added too many visualisations
        {
            QKeySequence keySeq( QString("%1").arg(shortcutNum++));
            newAction->setShortcut(keySeq);
        }   // end if

        _visualisationToolBar->insertAction( NULL, newAction);
        _viewGroup.addAction( newAction);
    }   // end foreach

    _visualisationToolBar->addSeparator();
    _visualisationToolBar->insertAction( NULL, ui->action_ShowBoundary);
    _visualisationToolBar->insertAction( NULL, ui->action_ShowLandmarks);
}   // end addActionsToVisualisationToolBar


// private
void FaceModelWidget::updateUI()
{
    const bool gotModels = ui->modelSelectorComboBox->count() > 0;
    ui->modelSelectorComboBox->setEnabled(gotModels);

    ui->action_ReorientModelLeft->setEnabled(gotModels);
    ui->action_ReorientModelFront->setEnabled(gotModels);
    ui->action_ReorientModelRight->setEnabled(gotModels);
    ui->leftOrientationButton->setEnabled(gotModels);
    ui->frontOrientationButton->setEnabled(gotModels);
    ui->rightOrientationButton->setEnabled(gotModels);

    ui->takeSnapshotButton->setEnabled(gotModels);
    ui->action_TakeSnapshot->setEnabled(gotModels);

    ui->action_ShowBoundary->setEnabled( gotModels && _boundaryViewer->getNumHandles() > 2);

    const FM* fm = getCurrentlySelectedModel( this);
    ui->action_ShowLandmarks->setEnabled( gotModels && fm->hasMarkers());

    _viewGroup.setEnabled(gotModels);
    _visualisationToolBar->setEnabled(gotModels);

    if ( fm)
    {
        if ( fm->getObjectModel()->getNumMaterials() == 0)
        {
            _viewGroup.actions().first()->setEnabled(false);
            if ( _viewGroup.actions().first()->isChecked())
                _viewGroup.actions().at(1)->setChecked(true);
        }   // end if
        else
            _viewGroup.actions().first()->setEnabled(true);   // Set texturing enabled by default
    }   // end if
}   // end updateUI


// public
FaceModelWidget::FaceModelWidget( const QActionGroup& viewGroup, QWidget *parent)
    : QWidget(parent), ui(new Ui::FaceModelWidget), _visualisationToolBar(NULL), _contextMenuHandler(NULL),
      _interactionManager(NULL), _imager(NULL), _surfaceMeasurer(NULL), _boundaryViewer(NULL),
      _leftMsgID(-1), _rightMsgID(-1), _visMsgID(-1), _currentlySelectedModelID(0), _viewGroup(this)
{
    ui->setupUi(this);
    _visualisationToolBar = new QToolBar(this);
    addActionsToVisualisationToolBar( viewGroup);
    connect( &_viewGroup, SIGNAL( triggered( QAction*)), this, SLOT( doOnApplyVisualisation( QAction*)));

    setButton( ui->takeSnapshotButton, ui->action_TakeSnapshot);
    setButton( ui->leftOrientationButton, ui->action_ReorientModelLeft);
    setButton( ui->frontOrientationButton, ui->action_ReorientModelFront);
    setButton( ui->rightOrientationButton, ui->action_ReorientModelRight);

    connect( ui->action_ReorientModelFront, SIGNAL( triggered()), this, SLOT( doOnSetCameraFrontProfile()));
    connect( ui->action_ReorientModelLeft, SIGNAL( triggered()), this, SLOT( doOnSetCameraLeftProfile()));
    connect( ui->action_ReorientModelRight, SIGNAL( triggered()), this, SLOT( doOnSetCameraRightProfile()));
    connect( ui->action_TakeSnapshot, SIGNAL( triggered()), this, SLOT( doOnTakeSnapshot()));
    connect( ui->action_ShowBoundary, SIGNAL( triggered(bool)), this, SLOT( doOnShowBoundary(bool)));

    _contextMenuHandler = new FaceApp::GUI::ModelViewerContextMenuHandler( this);
    connect( ui->modelViewer, SIGNAL( onShowContextMenu( const QPoint&)), this, SLOT( doOnShowContextMenu( const QPoint&)));
    connect( _contextMenuHandler, SIGNAL( onSetFocus()), this, SLOT( doOnSetFocus()));
    connect( _contextMenuHandler, SIGNAL( onDeleteMarker()), this, SLOT( doOnDeleteMarker()));

    _interactionManager = new FaceApp::InteractionManager( ui->modelViewer);
    _interactionManager->enableNonLiveModelHoverEvents(false);  // Only allow user interaction with the current model
    connect( _interactionManager, SIGNAL( onStartedMovingLandmark( int)), this, SLOT( doOnStartedMovingLandmark( int)));
    connect( _interactionManager, SIGNAL( onMovingLandmark( int)), this, SLOT( doOnMovingLandmark( int)));
    connect( _interactionManager, SIGNAL( onFinishedMovingLandmark( int)), this, SLOT( doOnFinishedMovingLandmark( int)));
    connect( _interactionManager, SIGNAL( onDoingMarkerHover(int)), this, SLOT( doOnDoingLandmarkHover(int)));
    connect( _interactionManager, SIGNAL( onFinishedMarkerHover(int)), this, SLOT( doOnFinishedLandmarkHover(int)));
    connect( _interactionManager, SIGNAL( onMousePressEvent()), this, SLOT( doOnMousePressEvent()));

    _imager = new ImageGrabber( ui->modelViewer->GetRenderWindow());

    _surfaceMeasurer = new SurfaceMeasurer( ui->modelViewer);
    connect( _interactionManager, SIGNAL( onStartedDrawingPath()), _surfaceMeasurer, SLOT( doOnStartedDrawingPath()));
    connect( _interactionManager, SIGNAL( onDrawingPath()), _surfaceMeasurer, SLOT( doOnDrawingPath()));
    connect( _interactionManager, SIGNAL( onFinishedDrawingPath()), _surfaceMeasurer, SLOT( doOnFinishedDrawingPath()));

    ui->action_ShowBoundary->setChecked(false);
    _boundaryViewer = new BoundaryViewer( ui->modelViewer->GetInteractor());
    connect( _boundaryViewer, SIGNAL( onBoundaryHandlesChanged()), this, SIGNAL( onBoundaryHandlesChanged()));

    ui->action_ShowLandmarks->setChecked(false);
    connect( ui->action_ShowLandmarks, SIGNAL( triggered(bool)), this, SLOT( doOnShowLandmarks(bool)));
    connect( LandmarksDialog::get(), SIGNAL( onShowLandmark(int, bool)), this, SLOT( doOnShowLandmark(int, bool)));

    _annotator = new FaceApp::Visualisation::ModelViewerAnnotator( ui->modelViewer->getRenderer());
    updateUI();
}   // end ctor


// public
FaceModelWidget::~FaceModelWidget()
{
    delete _annotator;
    delete _boundaryViewer;
    delete _surfaceMeasurer;
    delete _imager;
    delete _interactionManager;
    delete _contextMenuHandler;
    foreach ( QAction* action, _viewGroup.actions())
        delete action;
    delete _visualisationToolBar;
    delete ui;
}   // end dtor


// public
void FaceModelWidget::highlightFrame(bool visible)
{
    if ( visible)
        ui->innerFrame->setStyleSheet( "background-color: red;");
    else
        ui->innerFrame->setStyleSheet( "");
    //ui->innerFrame->setStyleSheet( "");
}   // end highlightFrame


// public
void FaceModelWidget::showMessageBottomLeft( const std::string &msg)
{
    _annotator->removeMessage( _leftMsgID);
    if ( !msg.empty())
        _leftMsgID = _annotator->showMessage( 0.05, 0.05, FaceApp::Visualisation::ModelViewerAnnotator::LeftJustify, msg);
    ui->modelViewer->updateRender();
}   // end showMessageBottomLeft


// public
void FaceModelWidget::showMessageBottomRight( const std::string &msg)
{
    _annotator->removeMessage( _rightMsgID);
    if ( !msg.empty())
        _rightMsgID = _annotator->showMessage( 0.95, 0.05, FaceApp::Visualisation::ModelViewerAnnotator::RightJustify, msg);
    ui->modelViewer->updateRender();
}   // end showMessageBottomRight


// private
void FaceModelWidget::showVisualisationTitle( const QString& msg)
{
    _annotator->removeMessage( _visMsgID);
    if ( !msg.isEmpty())
        _visMsgID = _annotator->showMessage( 0.97, 0.95, FaceApp::Visualisation::ModelViewerAnnotator::RightJustify, msg.toStdString());
}   // end showVisualisationTitle


// public
void FaceModelWidget::doOnAddModel( int modelID)
{
    addModel( modelID);
}   // end doOnAddModel


// public
void FaceModelWidget::doOnRemoveModel( int modelID)
{
    removeModelFromViewer( modelID);
}   // end doOnRemoveModel


// private
void FaceModelWidget::removeModelFromViewer( int modelID)
{
    if ( modelID > 0)
        ui->modelViewer->removeFaceModel( modelID);
}   // end removeModelFromViewer


// public
void FaceModelWidget::setCurrentModel( int modelID)
{
    removeModelFromViewer( getCurrentlySelectedComboBoxModelID());
    const int rowIndex = getComboBoxRowFromModelID( modelID);
    ui->modelSelectorComboBox->setCurrentIndex( rowIndex);  // Causes currentIndexChanged to fire
    _currentlySelectedModelID = modelID;
    _interactionManager->setLiveModelID( modelID);  // Set user interaction with the chosen model
    setCameraOrientationFromModel( modelID);  // Move camera before displaying model
    refreshCurrentModel();
}   // end setCUrrentModel


// public
void FaceModelWidget::refreshCurrentModel()
{
    updateUI();
    const int modelID = getCurrentlySelectedComboBoxModelID();
    _surfaceMeasurer->setModel( modelID);
    doOnShowBoundary( ui->action_ShowBoundary->isChecked());
    doOnShowLandmarks( ui->action_ShowLandmarks->isChecked());
    doOnApplyVisualisation( _viewGroup.checkedAction());
}   // end refreshCurrentModel


// public slot
void FaceModelWidget::doOnApplyVisualisation( QAction* visAction)
{
    if ( visAction == NULL)
        visAction = _viewGroup.actions().first();
    visAction->setChecked(true);
    VisualisationInterface* vint = dynamic_cast<VisualisationInterface*>(visAction->parent());
    QString visTitle = vint->getDisplayName();
    if ( _currentlySelectedModelID <= 0)
        visTitle = "";
    showVisualisationTitle( visTitle);
    ui->modelViewer->applyVisualisation( vint, _currentlySelectedModelID);
    updateUI();
}   // end doOnApplyVisualisation


// public slot
void FaceModelWidget::doOnShowBoundary( bool show)
{
    _boundaryViewer->setModel( getCurrentlySelectedComboBoxModelID());
    const FM* fm = getCurrentlySelectedModel(this);
    show &= fm && !fm->getBoundaryHandles().empty();
    ui->action_ShowBoundary->setChecked( show);
    _boundaryViewer->showBoundary( show);
    ui->modelViewer->updateRender();
}   // end doOnShowBoundary


// public slot
void FaceModelWidget::doOnShowLandmarks( bool show)
{
    if ( getCurrentlySelectedComboBoxModelID() == 0)
        return;
    const FM* fm = getCurrentlySelectedModel( this);
    show &= fm && fm->hasMarkers();
    ui->action_ShowLandmarks->setChecked( show);
    const IntSet& mids = fm->getMarkerIds();
    foreach ( const int& mid, mids)
        doOnShowLandmark( mid, show);
}   // end doOnShowLandmarks


// public
void FaceModelWidget::setCameraOrientationFromModel( int modelID)
{
    if ( modelID > 0)
    {
        const FM* fm = FMM::getModel(modelID);
        FaceApp::CameraParams cp = FaceApp::CameraParams( fm);
        ui->modelViewer->setCamera(cp);
    }   // end if
}   // end setCameraOrientationFromModel


// public
void FaceModelWidget::getCamera( FaceApp::CameraParams* cp) const
{
    assert(cp);
    *cp = ui->modelViewer->getCameraParams();
}   // end getCamera


// protected
void FaceModelWidget::mousePressEvent( QMouseEvent *event) // Handles mouse presses in the area outside the ModelViewer
{
    event->setAccepted(false);   // Don't propagate to parent
    doOnMousePressEvent();
}   // end mousePressEvent


// protected
void FaceModelWidget::resizeEvent(QResizeEvent *event)
{
    _annotator->doOnUpdateMessagePositions();
}   // end resizeEvent


// private slot
void FaceModelWidget::doOnMousePressEvent()
{
    emit onReceivedMousePressEvent(this);
}   // end doOnMousePressEvent


// public
size_t FaceModelWidget::getAllBoundaryVertices( std::vector<cv::Vec3f>& bverts) const
{
    assert( getCurrentlySelectedComboBoxModelID() > 0);
    return _boundaryViewer->getVertices( bverts);
}   // end getAllBoundaryVertices


// public
size_t FaceModelWidget::getBoundaryHandles( std::vector<cv::Vec3f>& bhandles) const
{
    assert( getCurrentlySelectedComboBoxModelID() > 0);
    return _boundaryViewer->getHandles( bhandles);
}   // end getBoundaryHandles


// public
cv::Point FaceModelWidget::projectToPlane( const cv::Vec3f &wv, cv::Size imageDims) const
{
    if (imageDims.width == 0 && imageDims.height == 0)
        imageDims = ui->modelViewer->getSize();
    const cv::Point2f pf = ui->modelViewer->projectToDisplayProportion(wv);
    return FaceApp::fromProportion( pf, imageDims);
}   // end projectToPlane


// public
const ImageGrabber& FaceModelWidget::getUpdatedImages( bool showLandmarks, bool showBoundary)
{
    if ( getCurrentlySelectedComboBoxModelID() > 0)
    {
        const bool oldLandmarksState = ui->action_ShowLandmarks->isChecked();
        const bool oldBoundaryState = ui->action_ShowBoundary->isChecked();
        doOnShowBoundary( showBoundary);
        doOnShowLandmarks( showLandmarks);
        _imager->update( IMG_GRAB_ROWS);
        doOnShowBoundary( oldBoundaryState);
        doOnShowLandmarks( oldLandmarksState);
    }   // end if
    return *_imager;
}   // end getUpdatedImages


// public
bool FaceModelWidget::calcSurfacePosition( int modelID, const cv::Point2f &p, cv::Vec3f& worldPos) const
{
    assert(_modelIDs.count(modelID));
    if ( !_modelIDs.count(modelID))
        return false;
    return ui->modelViewer->calcSurfacePosition( modelID, p, worldPos);
}   // end calcSurfacePosition


// private slot
void FaceModelWidget::doOnShowContextMenu( const QPoint& p)
{
    const int modelID = ui->modelViewer->getPointedAtModelID();
    if ( modelID <= 0)
        return;

    // Model ID selected graphically must match the one from the dropdown list
    if ( modelID != getCurrentlySelectedComboBoxModelID())
        return;

    _contextMenuHandler->enableSetFocus( true);
    const int mid = ui->modelViewer->getPointedAtMarkerID();
    if ( mid >= 0 && LandmarksDialog::canDeleteLandmark( mid))
    {
        const FM* fm = FMM::getModel( modelID);
        _contextMenuHandler->allowUserDeleteMarker( fm->getMarker(mid)->getName());
    }   // end if
    else
        _contextMenuHandler->allowUserDeleteMarker( "");
    _contextMenuHandler->showContextMenu( ui->modelViewer->mapToGlobal(p));
}   // end doOnShowContextMenu


// private slot
void FaceModelWidget::doOnDeleteMarker()
{
    const FM* fm = getCurrentlySelectedModel(this);
    assert(fm);
    // Get the marker being pointed to
    const int mid = ui->modelViewer->getPointedAtMarkerID();
    if ( mid >= 0 && fm->getMarker( mid))
    {
        ui->modelViewer->removeMarker( mid);
        emit onDeleteMarker( mid);
    }   // end if
}   // end doOnDeleteMarker


// private slot
void FaceModelWidget::doOnStartedMovingLandmark( int mid)
{
    const FM* fm = getCurrentlySelectedModel(this);
    showMessageBottomRight( fm->getMarker(mid)->getName());
}   // end doOnStartedMovingLandmark


// private slot
void FaceModelWidget::doOnMovingLandmark( int mid)
{
    const FM* fm = getCurrentlySelectedModel( this);
    assert(fm);
    cv::Vec3f wv;
    const cv::Point2f mp = ui->modelViewer->getProportionateMouseCoords();
    if ( ui->modelViewer->calcSurfacePosition( fm->getId(), mp, wv))
    {
        emit onUpdateMarkerPosition( mid, wv);
        ui->modelViewer->setMarkerPos( mid, wv);
        LandmarksDialog::updateLandmark( mid);
        showMessageBottomRight( fm->getMarker(mid)->getName());
    }   // end if
}   // end doOnMovingLandmark


// private slot
void FaceModelWidget::doOnFinishedMovingLandmark( int mid)
{
    LandmarksDialog::updateLandmark( mid);
    showMessageBottomRight();
}   // end doOnFinishedMovingLandmark


// private slot
void FaceModelWidget::doOnDoingLandmarkHover( int mid)
{
    const FM* fm = getCurrentlySelectedModel( this);
    showMessageBottomRight( fm->getMarker(mid)->getName());
}   // end doOnDoingLandmarkHover


// private slot
void FaceModelWidget::doOnFinishedLandmarkHover( int /*notused*/)
{
    showMessageBottomRight();
}   // end doOnFinishedLandmarkHover


// public slot
void FaceModelWidget::doOnSetCameraFrontProfile()
{
    const int modelID = getCurrentlySelectedComboBoxModelID();
    if ( modelID > 0)
    {
        setCameraOrientationFromModel( modelID);
        ui->modelViewer->updateRender();
    }   // end if
}   // end doOnSetCameraFrontProfile


// public slot
void FaceModelWidget::doOnSetCameraLeftProfile()
{
    const int modelID = getCurrentlySelectedComboBoxModelID();
    if ( modelID > 0)
    {
        const FM* fm = FMM::getModel(modelID);
        FaceApp::CameraParams cp = FaceApp::CameraParams( fm);
        cp.rotateAboutUpAxis(70);
        ui->modelViewer->setCamera(cp);
        ui->modelViewer->updateRender();
    }   // end if
}   // end doOnSetCameraLeftProfile


// public slot
void FaceModelWidget::doOnSetCameraRightProfile()
{
    const int modelID = getCurrentlySelectedComboBoxModelID();
    if ( modelID > 0)
    {
        const FM* fm = FMM::getModel(modelID);
        FaceApp::CameraParams cp = FaceApp::CameraParams( fm);
        cp.rotateAboutUpAxis(-70);
        ui->modelViewer->setCamera(cp);
        ui->modelViewer->updateRender();
    }   // end if
}   // end doOnSetCameraRightProfile


// public slot
void FaceModelWidget::doOnPrintCameraDetails()
{
    RVTK::printCameraDetails( ui->modelViewer->getRenderer()->GetActiveCamera(), std::cerr);
}   // end doOnPrintCameraDetails


// private slot
void FaceModelWidget::doOnShowLandmark( int mid, bool show)
{
    const FM* fm = getCurrentlySelectedModel( this);
    assert( fm->getMarker(mid));
    show &= ui->action_ShowLandmarks->isChecked() && LandmarksDialog::canSeeLandmark(mid) &&
            (!fm->getMarker(mid)->isDebug() || DebugDialog::getShowDebugVertices());
    if ( show)
        ui->modelViewer->addMarker( fm->getMarker(mid));
    else
        ui->modelViewer->removeMarker( mid);
    ui->modelViewer->updateRender();
}   // end doOnShowLandmark


// public slot
void FaceModelWidget::doOnSetFocus()
{
    const int modelID = getCurrentlySelectedComboBoxModelID();
    const cv::Point2f pf = ui->modelViewer->getProportionateMouseCoords();
    cv::Vec3f wv;
    if ( ui->modelViewer->calcSurfacePosition( modelID, pf, wv))
    {
        FaceApp::CameraParams cp;
        ui->modelViewer->getCameraPosition( cp.pos);
        cp.focus = wv;
        ui->modelViewer->getCameraViewUp( cp.up);
        ui->modelViewer->setCamera( cp);
        ui->modelViewer->updateRender();
    }   // end if
}   // end doOnSetFocus

