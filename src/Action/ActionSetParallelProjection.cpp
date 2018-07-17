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

#include <ActionSetParallelProjection.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <algorithm>
using FaceTools::Action::ActionSetParallelProjection;
using FaceTools::Action::FaceAction;
using FaceTools::FaceControlSet;
using FaceTools::ModelViewer;


ActionSetParallelProjection::ActionSetParallelProjection( const QString& dn, const QIcon& ico)
    : FaceAction( dn, ico)
{
    setCheckable(true,false);
}   // end ctor


bool ActionSetParallelProjection::doAction( FaceControlSet&)
{
    for ( ModelViewer* viewer : _viewers)
    {
        vtkCamera* cam = const_cast<vtkRenderer*>(viewer->getRenderer())->GetActiveCamera();
        if ( isChecked())
        {
            cam->ParallelProjectionOn();
            RFeatures::CameraParams cp = viewer->getCamera();
            const double pflen = viewer->getHeight() / tan(cp.fovRads()/2); // The pixel focal length
            const double ratio = viewer->getHeight() / pflen;
            const double D = viewer->cameraDistance(); // Distance to focal point (world coords)
            cam->SetParallelScale( D*ratio);
        }   // end if
        else
            cam->ParallelProjectionOff();
        viewer->updateRender();
    }   // end else
    return true;
}   // end doAction
