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

#include <FaceModelViewer.h>
#include <VisualisationAction.h>
#include <ImageGrabber.h>   // RVTK
#include <vtkInteractorStyleTrackballCamera.h>
#include <QVBoxLayout>
#include <QMenu>
using FaceTools::FaceModel;
using FaceTools::FaceControl;
using FaceTools::ModelSelector;
using FaceTools::FaceModelViewer;

namespace
{

typedef std::pair<FaceModel*, FaceControl*> MPair;

}   // end namespace


// public
FaceModelViewer::FaceModelViewer( QMenu* cmenu, QWidget *parent)
    : QWidget(parent), _cmenu(cmenu),
      _viewer(NULL), _selector(NULL),
      _fsaction(NULL),
      _screenshotSaver( "Save Screenshot", this),
      _cameraResetter( "Reset Camera", this),
      _axesToggler( "Toggle Axes", this)
{
    QTools::VtkActorViewer* qviewer = new QTools::VtkActorViewer;
    qviewer->setInteractor( vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    _viewer = new FaceTools::InteractiveModelViewer( qviewer);
    _selector = new ModelSelector( _viewer);
    _fsaction = new FaceTools::ActionFullScreenViewer( _viewer);
    _fsaction->qaction()->setEnabled(false);   // DISABLE FULLSCREEN FOR NOW (NOT WORKING PROPERLY)
    _axesToggler.setCheckable(true);
    _axesToggler.setChecked( _viewer->axesShown());

    setLayout( new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    _viewer->addToLayout( layout());
    connect( _viewer, &FaceTools::InteractiveModelViewer::requestContextMenu, this, &FaceModelViewer::showContextMenu);
    connect( _selector, &ModelSelector::onSelected, this, &FaceModelViewer::doOnSelected);
    connect( &_screenshotSaver, &QAction::triggered, [=](){ _viewer->saveSnapshot();});
    connect( &_cameraResetter, &QAction::triggered,  [=](){ _viewer->resetDefaultCamera( 650.0f); _viewer->updateRender();});
    connect( &_axesToggler, &QAction::triggered,     [=](){ _viewer->showAxes( _axesToggler.isChecked()); _viewer->updateRender();});

    _cameraResetter.trigger();
}   // end ctor


// public
FaceModelViewer::~FaceModelViewer()
{
    delete _fsaction;
    delete _selector;
    delete _viewer;
}   // end dtor


// public
size_t FaceModelViewer::getModels( boost::unordered_set<FaceModel*>& fmodels) const
{
    foreach ( const MPair& mp, _modelConts)
        fmodels.insert(mp.first);
    return getNumModels();
}   // end getModels


// public
void FaceModelViewer::updateRender() { _viewer->updateRender();}
void FaceModelViewer::addKeyPressHandler( QTools::KeyPressHandler* kph) { _viewer->addKeyPressHandler(kph);}
void FaceModelViewer::removeKeyPressHandler( QTools::KeyPressHandler* kph) { _viewer->removeKeyPressHandler(kph);}


// public
void FaceModelViewer::applyOptions( const FaceTools::ModelOptions& opts)
{
    const boost::unordered_set<FaceControl*>& uselected = _selector->getSelected();
    foreach ( FaceControl* fcont, uselected)
        fcont->setOptions(opts);
    _viewer->showAxes( opts.showAxes);
    _viewer->updateRender();
}   // end applyOptions


// public
cv::Mat_<cv::Vec3b> FaceModelViewer::grabImage() const
{
    RVTK::ImageGrabber imgGrabber( _viewer->getRenderWindow());
    imgGrabber.update();
    return imgGrabber.getColourMap();
}   // end grabImage


size_t FaceModelViewer::getNumModels() const { return _modelConts.size();}


// public
size_t FaceModelViewer::getSelectedModels( boost::unordered_set<FaceModel*>* fms) const
{
    if ( fms)
    {
        const boost::unordered_set<FaceControl*>& uselected = _selector->getSelected();
        foreach ( FaceControl* fcont, uselected)
            fms->insert( fcont->getModel());
    }   // end if
    return _selector->getSelected().size();
}   // end getSelectedModels


// public
bool FaceModelViewer::give( FaceControl* fcont)
{
    // Don't add view if its model is already in the viewer.
    if ( _modelConts.count(fcont->getModel()) > 0)
        return false;
    if ( _modelConts.empty())
        _cameraResetter.trigger();
    _modelConts[fcont->getModel()] = fcont;
    fcont->setViewer(_viewer);
    _viewer->connectInterface(fcont);
    fcont->setVisualisation( fcont->getVisualisation());
    _selector->add(fcont);  // Auto selects on add
    setControlled( fcont->getModel(), true, true);
    return true;
}   // end give


// public
FaceControl* FaceModelViewer::take( FaceModel* fmodel)
{
    assert(fmodel);
    if ( _modelConts.count(fmodel) == 0)
        return NULL;
    FaceControl* fcont = _modelConts.at(fmodel);
    _selector->remove(fcont);   // Auto deselects on remove
    setControlled( fmodel, false, true);
    _viewer->disconnectInterface(fcont);
    _modelConts.erase(fmodel);
    fcont->setViewer(NULL);
    if ( _modelConts.empty())
        _cameraResetter.trigger();
    return fcont;
}   // end take


// public
FaceControl* FaceModelViewer::get( FaceModel* fmodel) const
{
    if ( _modelConts.count(fmodel) > 0)
        return _modelConts.at(fmodel);
    return NULL;
}   // end get


// protected
void FaceModelViewer::resizeEvent( QResizeEvent* evt)
{
    const int oldsize = evt->oldSize().width() * evt->oldSize().height();
    const int newsize = evt->size().width() * evt->size().height();
    if ( newsize == 0 && oldsize > 0)
        emit toggleZeroArea( true);
    else if ( newsize > 0 && oldsize == 0)
        emit toggleZeroArea( false);
}   // end resizeEvent


// public
bool FaceModelViewer::setControlled( FaceModel* fmodel, bool controlled, bool emitSelected)
{
    if ( _modelConts.count(fmodel) == 0)
        return false;
    FaceControl* fcont = _modelConts.at(fmodel);
    _viewer->enableFloodLights( fcont->getVisualisation()->useTexture());
    _selector->setSelected( fcont, controlled);
    fcont->setControlled(controlled);    // Causes actions to fire
    fcont->showSelected(controlled);
    if ( emitSelected)
        emit updatedSelected( fcont, controlled);
    return true;
}   // end setControlled


// private slot
void FaceModelViewer::doOnSelected( FaceControl* fcont, bool v)
{
    setControlled( fcont->getModel(), v, true);
}   // end doOnSelected


// private slot
void FaceModelViewer::showContextMenu( const QPoint& p)
{
    // Only show if p is on one of the models currently selected
    bool onModel = false;
    const boost::unordered_set<FaceControl*>& uselected = _selector->getSelected();
    foreach ( const FaceControl* fcont, uselected)
    {
        if ( onModel = fcont->isPointedAt())
            break;
    }   // end foreach
    if ( onModel)
        _cmenu->exec(mapToGlobal(p));
}   // end showContextMenu
