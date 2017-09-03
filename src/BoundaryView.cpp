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

#include <BoundaryView.h>
using FaceTools::BoundaryViewEventObserver;
using FaceTools::BoundaryView;
using FaceTools::ObjMetaData;
#include <sstream>
#include <cassert>


// public virtual
void BoundaryViewEventObserver::interactionEvent( const RVTK::ModelPathDrawer* boundary)
{
    std::vector<cv::Vec3f> bvecs;
    boundary->getAllPathVertices( bvecs);
    emit updatedBoundary( bvecs);
}   // end interactionEvent


// public
BoundaryView::BoundaryView( ModelViewer* viewer, const ObjMetaData::Ptr objmeta, BoundaryViewEventObserver* observer)
    : _objmeta(objmeta)
{
    _boundary = RVTK::ModelPathDrawer::create( viewer->getRenderWindow()->GetInteractor());
    _boundary->addEventObserver( observer);
    _boundary->setClosed( true);
}   // end ctor


void BoundaryView::show( bool enable) { _boundary->setVisibility(enable); }
bool BoundaryView::isShown() const { return _boundary->getVisibility();}

void BoundaryView::allowAdjustment( bool enable) { _boundary->setProcessEvents(enable);}
bool BoundaryView::canAdjust() const { return _boundary->getProcessEvents();}


// public
void BoundaryView::reset( const vtkActor* actor)
{
    assert(actor);
    std::vector<cv::Vec3f> bhandles;
    const bool madeHandles = _objmeta->makeBoundaryHandles( bhandles);
    assert(madeHandles);
    _boundary->setPathHandles( bhandles); // Update the interpolated path over the surface of the face
    _boundary->setModel( actor);
}   // end reset


// public
void BoundaryView::setVisualisationOptions( const VisualisationOptions::Boundary& visopts)
{
    _visopts = visopts;
    _boundary->setLineWidth( _visopts.lineWidth);
    _boundary->setPointSize( _visopts.vertexSize);

    const QColor& lcol = _visopts.lineColour;
    _boundary->setLineColour( lcol.redF(), lcol.greenF(), lcol.blueF());

    const QColor& vcol = _visopts.vertexColour;
    _boundary->setPointColour( vcol.redF(), vcol.greenF(), vcol.blueF());
}   // end setVisualisationOptions


