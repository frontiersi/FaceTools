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

#include <FaceActions.h>
#include <FaceModel.h>
#include <FaceView.h>


bool FaceTools::DeleteLandmarkAction::operator()( FaceView* fview, const QPoint& p)
{
}   // end operator()


bool FaceTools::AddLandmarkAction::operator()( FaceView* fview, const QPoint& p)
{
}   // end operator()


bool FaceTools::SetFocusAction::operator()( FaceView* fview, const QPoint& p)
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
}   // end operator()


bool FaceTools::ToggleShowBoundaryAction::operator()( FaceView* fview, const QPoint& p)
{
}   // end operator()


bool FaceTools::ToggleShowLandmarksAction::operator()( FaceView* fview, const QPoint& p)
{
}   // end operator()



bool FaceTools::CropBoundaryAction::operator()( FaceModel* fmodel, const cv::Vec3f* v)
{
}   // end operator()
