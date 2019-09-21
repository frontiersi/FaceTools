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

#include <Action/ActionSetParallelProjection.h>
#include <FileIO/FaceModelManager.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <algorithm>
using FaceTools::Action::ActionSetParallelProjection;
using FaceTools::Action::FaceAction;
using FaceTools::Action::Event;
using FaceTools::Vis::FV;
using FaceTools::ModelViewer;
using MS = FaceTools::Action::ModelSelector;


ActionSetParallelProjection::ActionSetParallelProjection( const QString& dn, const QIcon& ico, const QKeySequence& ks)
    : FaceAction( dn, ico, ks)
{
    setCheckable(true,false);
    addTriggerEvent(Event::CLOSED_MODEL);
}   // end ctor


bool ActionSetParallelProjection::checkState( Event)
{
    if ( FileIO::FMM::numOpen() == 0)
        return false;
    return isChecked();
}   // end checkChecked


bool ActionSetParallelProjection::checkEnable( Event)
{
    return FileIO::FMM::numOpen() > 0;
}   // end checkEnabled


void ActionSetParallelProjection::doAction( Event)
{
    for ( FMV* fmv : MS::viewers())
    {
        vtkCamera* cam = const_cast<vtkRenderer*>(fmv->getRenderer())->GetActiveCamera();
        if ( isChecked())
        {
            cam->ParallelProjectionOn();
            RFeatures::CameraParams cp = fmv->camera();
            const double pflen = fmv->getHeight() / tan(cp.fovRads()/2); // The pixel focal length
            const double ratio = fmv->getHeight() / pflen;
            const double D = fmv->cameraDistance(); // Distance to focal point (world coords)
            cam->SetParallelScale( D*ratio);
        }   // end if
        else
            cam->ParallelProjectionOff();
    }   // end for
}   // end doAction
