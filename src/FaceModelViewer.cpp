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
#include <FaceControl.h>
#include <FaceModel.h>
#include <QVBoxLayout>
#include <cassert>
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;
using FaceTools::Interactor::ModelSelectInteractor;

// public
FaceModelViewer::FaceModelViewer( QWidget *parent, bool exs)
    : QWidget(parent), FaceTools::ModelViewer(), _selector( NULL)
{
    setLayout( new QVBoxLayout);
    layout()->setContentsMargins( QMargins(0,0,0,0));
    addToLayout( layout());

    resetCamera();  // Default camera on instantiation
    _selector = new ModelSelectInteractor( this, exs);
    connect( _selector, &ModelSelectInteractor::onUserSelected, this, &FaceModelViewer::onUserSelected);
}   // end ctor


// public
FaceModelViewer::~FaceModelViewer() { delete _selector;}


// public slots
void FaceModelViewer::saveScreenshot() const { saveSnapshot();}
void FaceModelViewer::resetCamera() { resetDefaultCamera( 650.0f); updateRender();}
// public
const FaceControlSet& FaceModelViewer::selected() const { return _selector->selected();}
const FaceControlSet& FaceModelViewer::attached() const { return _selector->available();}
bool FaceModelViewer::isSelected( FaceControl* fc) const { return _selector->isSelected(fc);}
bool FaceModelViewer::isAttached( FaceControl* fc) const { return _selector->isAvailable(fc);}


// public
bool FaceModelViewer::attach( FaceControl* fcont)
{
    assert(fcont);
    if ( _attached.count(fcont->data()) > 0) // Don't add view if its model is already in the viewer.
        return false;
    if ( _attached.empty())
        resetCamera();
    _attached[fcont->data()] = fcont;
    fcont->setViewer( this);
    _selector->add(fcont);
    emit onUserSelected( fcont, true);
    return true;
}   // end attach


// public
bool FaceModelViewer::detach( FaceControl* fcont)
{
    if ( fcont == NULL || _attached.count(fcont->data()) == 0)
        return false;
    _selector->remove(fcont);
    emit onUserSelected( fcont, false);
    _attached.erase(fcont->data());
    fcont->setViewer(NULL);
    if ( _attached.empty())   // Reset camera to default if now empty
        resetCamera();
    return true;
}   // end detach


// public
FaceControl* FaceModelViewer::get( FaceTools::FaceModel* fmodel) const
{
    if ( _attached.count(fmodel) > 0)
       return _attached.at(fmodel);
    return NULL;
}   // end get


// public
void FaceModelViewer::setSelected( FaceControl* fc, bool selected)
{
    assert(fc);
    // Only FaceControl instances currently attached to this viewer may be selected.
    if ( attached().count(fc) == 0)
        return;

    assert( _attached.count(fc->data()) > 0);
    if ( isSelected(fc) != selected)    // Only change if needing to.
    {
        _selector->setSelected( fc, selected);
        emit onUserSelected( fc, selected);
    }   // end if
}   // end setSelected


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
