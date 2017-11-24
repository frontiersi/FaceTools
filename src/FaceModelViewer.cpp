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
#include <vtkInteractorStyleTrackballCamera.h>
#include <QVBoxLayout>
#include <QMenu>
using FaceTools::FaceModel;
using FaceTools::FaceControl;
using FaceTools::ModelSelector;
using FaceTools::FaceModelViewer;

namespace {

typedef std::pair<FaceModel*, FaceControl*> MPair;


FaceControl* findViewFromProp( const boost::unordered_map<FaceModel*, FaceControl*> mfcont, const vtkProp* p)
{
    // Search for the model - could hash the props in to make this faster,
    // but since the operation is user driven, and there won't be that many
    // models to search through, this is okay for now.
    FaceControl* sfcont = NULL; // Will be the selected FaceControl.
    foreach ( const MPair& mp, mfcont)
    {
        FaceControl* fcont = mp.second;
        if ( fcont->belongs(p))
        {
            sfcont = fcont;
            break;
        }   // end if
    }   // end foreach
    return sfcont;
}   // end findViewFromProp

}   // end namespace


// public
FaceModelViewer::FaceModelViewer( QMenu* cmenu, QWidget *parent)
    : QWidget(parent), _cmenu(cmenu),
      _viewer(NULL), _selector(NULL),
      _fsaction(NULL),
      _screenshotSaver( "Save Screenshot", this),
      _cameraResetter( "Reset Camera", this),
      _lightsToggler( "Toggle Texture Lighting", this),
      _axesToggler( "Toggle Axes", this)
{
    QTools::VtkActorViewer* qviewer = new QTools::VtkActorViewer;
    qviewer->setInteractor( vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New());
    _viewer = new FaceTools::InteractiveModelViewer( qviewer);
    _selector = new ModelSelector( _viewer);
    _fsaction = new FaceTools::ActionFullScreenViewer( _viewer);
    _fsaction->qaction()->setEnabled(false);   // DISABLE FULLSCREEN FOR NOW (NOT WORKING PROPERLY)
    _lightsToggler.setCheckable(true);
    _lightsToggler.setChecked( _viewer->floodLightsEnabled());
    _axesToggler.setCheckable(true);
    _axesToggler.setChecked( _viewer->axesShown());

    setLayout( new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    _viewer->addToLayout( layout());
    connect( _viewer, &FaceTools::InteractiveModelViewer::requestContextMenu, this, &FaceModelViewer::showContextMenu);
    connect( _selector, &ModelSelector::onSelectAll, this, &FaceModelViewer::doOnSelectAll);
    connect( _selector, &ModelSelector::onSelected, this, &FaceModelViewer::toggleControlled);
    connect( &_screenshotSaver, &QAction::triggered, [=](){ _viewer->saveSnapshot();});
    connect( &_cameraResetter, &QAction::triggered,  [=](){ _viewer->resetDefaultCamera( 650.0f); _viewer->updateRender();});
    connect( &_lightsToggler, &QAction::triggered,   [=](){ _viewer->enableFloodLights( _lightsToggler.isChecked()); _viewer->updateRender();});
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


size_t FaceModelViewer::getNumModels() const { return _modelConts.size();}


// public
size_t FaceModelViewer::getSelectedModels( boost::unordered_set<FaceModel*>* fms) const
{
    if ( fms)
    {
        foreach ( FaceControl* fcont, _uselected)
            fms->insert( fcont->getModel());
    }   // end if
    return _uselected.size();
}   // end getSelectedModels


// public
bool FaceModelViewer::give( FaceControl* fcont)
{
    // Don't add view if its model is already in the viewer.
    if ( _modelConts.count(fcont->getModel()) > 0)
        return false;
    _modelConts[fcont->getModel()] = fcont;
    _viewer->connectInterface(fcont);
    fcont->setViewer(_viewer);
    fcont->setVisualisation( fcont->getVisualisation());
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
    setControlled( fcont->getModel(), false, true);
    _viewer->disconnectInterface(fcont);
    _modelConts.erase(fmodel);
    fcont->setViewer(NULL);
    return fcont;
}   // end take


// public
FaceControl* FaceModelViewer::get( FaceModel* fmodel) const
{
    if ( _modelConts.count(fmodel) > 0)
        return _modelConts.at(fmodel);
    return NULL;
}   // end get


// public
void FaceModelViewer::setAllControlled( bool enable)
{
    foreach ( const MPair& mp, _modelConts)
        setControlled( mp.first, enable, false);
}   // end setAllControlled


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
    fcont->setControlled(controlled);    // Causes actions to fire
    fcont->showOutline(controlled);
    if ( controlled)
        _uselected.insert( fcont);
    else
        _uselected.erase( fcont);
    if ( emitSelected)
        emit updatedSelected( fcont, controlled);
    return true;
}   // end setControlled


// private slot
void FaceModelViewer::toggleControlled( const vtkProp* p, bool)
{
    FaceControl* fcont = findViewFromProp( _modelConts, p);
    if ( fcont)
        setControlled( fcont->getModel(), _uselected.count(fcont) == 0, true); // Toggle interactivity on the activated model
}   // end toggleControlled


// private slot
void FaceModelViewer::doOnSelectAll()
{
    // Select all if not all currently selected, otherwise clear.
    const bool selectAll = _uselected.size() < _modelConts.size();
    foreach ( const MPair& mp, _modelConts)
        setControlled( mp.first, selectAll, true);
}   // end doOnSelectAll


// private slot
void FaceModelViewer::showContextMenu( const QPoint& p)
{
    // Only show if p is on one of the models currently selected
    bool onModel = false;
    foreach ( const FaceControl* fcont, _uselected)
    {
        if ( onModel = fcont->isPointedAt())
            break;
    }   // end foreach
    if ( onModel)
        _cmenu->exec(mapToGlobal(p));
}   // end showContextMenu
