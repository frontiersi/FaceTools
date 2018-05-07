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
#include <cassert>
using FaceTools::FaceModelViewer;
using FaceTools::FaceControlSet;
using FaceTools::FaceControl;

FaceModelViewer::FaceModelViewer( QWidget *parent) : ModelViewer(parent)
{
    resetCamera();  // Default camera on instantiation
}   // end ctor


void FaceModelViewer::saveScreenshot() const { saveSnapshot();}
void FaceModelViewer::resetCamera() { resetDefaultCamera( 650.0f); updateRender();}


bool FaceModelViewer::attach( FaceControl* fc)
{
    if ( _models.count(fc->data()) > 0) // Don't add view if its model is already in the viewer.
        return false;
    if ( _attached.empty())
        resetCamera();
    _attached.insert(fc);
    _models[fc->data()] = fc;
    fc->setViewer( this);
    return true;
}   // end attach


bool FaceModelViewer::detach( FaceControl* fc)
{
    if ( fc == NULL || !_attached.has(fc))
        return false;
    _attached.erase(fc);
    _models.erase(fc->data());
    fc->setViewer(NULL);
    if ( _attached.empty())   // Reset camera to default if now empty
        resetCamera();
    return true;
}   // end detach


FaceControl* FaceModelViewer::get( FaceTools::FaceModel* fm) const
{
    if ( _models.count(fm) == 0)
        return NULL;
    return _models.at(fm);
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
