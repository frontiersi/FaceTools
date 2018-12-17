/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
#include <FaceModel.h>
#include <FaceView.h>
#include <algorithm>
#include <cassert>
using FaceTools::FaceModelViewer;
using FaceTools::FVFlags;
using FaceTools::Vis::FV;
using FaceTools::FM;

FaceModelViewer::FaceModelViewer( QWidget *parent) : ModelViewer(parent)
{
    resetCamera();  // Default camera on instantiation
}   // end ctor


void FaceModelViewer::saveScreenshot() const { saveSnapshot();}
void FaceModelViewer::resetCamera() { resetDefaultCamera( 650.0f);}


bool FaceModelViewer::attach( FV* fv)
{
    if ( _models.count(fv->data()) > 0) // Don't add view if its model is already in the viewer.
        return false;
    _attached.insert(fv);
    _models[fv->data()] = fv;
    emit onAttached(fv);
    return true;
}   // end attach


bool FaceModelViewer::detach( FV* fv)
{
    if ( fv == nullptr || !_attached.has(fv))
        return false;
    _attached.erase(fv);
    _models.erase(fv->data());
    emit onDetached(fv);
    return true;
}   // end detach


FV* FaceModelViewer::get( const FM* fm) const
{
    if ( _models.count(fm) == 0)
        return nullptr;
    return _models.at(fm);
}   // end get


size_t FaceModelViewer::findOverlaps( FVFlags &olaps) const
{
    std::vector<FV*> fvs( _attached.begin(), _attached.end()); // Make ordered for comparison
    const size_t n = fvs.size();

    // Initialise all to false.
    for ( size_t i = 0; i < n; ++i)
        olaps[fvs[i]] = false;

    size_t nol = 0;
    for ( size_t i = 0; i < n; ++i)
    {
        FV* ifv = fvs[i];
        for ( size_t j = i+1; j < n; ++j)
        {
            FV* jfv = fvs[j];
            if ( ifv->data()->supersIntersect( *jfv->data()))
            {
                olaps[ifv] = olaps[jfv] = true;
                nol++;
            }   // end if
        }   // end for
    }   // end for

    return nol;
}   // end findOverlaps


void FaceModelViewer::refreshOverlapOpacity( const FVFlags& olaps, double maxOpacityOnOverlap) const
{
    for ( FV* fv : _attached)
    {
        const double olapVal = std::min( fv->opacity(), maxOpacityOnOverlap);
        fv->setOpacity( olaps.at(fv) ? olapVal : 1.0);
    }   // end for
}   // end refreshOpacity


// protected
void FaceModelViewer::resizeEvent( QResizeEvent* evt)
{
    const int oldsize = evt->oldSize().width() * evt->oldSize().height();
    const int newsize = evt->size().width() * evt->size().height();
    ModelViewer::resizeEvent(evt);
    if ( newsize == 0 && oldsize > 0)
        emit toggleZeroArea( true);
    else if ( newsize > 0 && oldsize == 0)
        emit toggleZeroArea( false);
}   // end resizeEvent
