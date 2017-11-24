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

#include <ActionOrientCameraToFace.h>
#include <InteractiveModelViewer.h>
#include <FaceTools.h>
using FaceTools::ActionOrientCameraToFace;
using FaceTools::FaceControl;
using FaceTools::ObjMetaData;


ActionOrientCameraToFace::ActionOrientCameraToFace( const std::string& dname, const std::string& fname)
    : FaceTools::ActionProcessModel( dname, fname, true, true, true)
{
}   // end ctor


bool ActionOrientCameraToFace::operator()( FaceControl* fcont)
{
    assert( isActionable( fcont));
    const ObjMetaData::Ptr omd = fcont->getModel()->getObjectMeta();
    const cv::Vec3f focus = FaceTools::calcFaceCentre(omd);
    cv::Vec3f nvec, uvec;
    if ( !omd->getOrientation( nvec, uvec))
    {
        std::cerr << "Unable to get orientation from ObjMetaData!" << std::endl;
        assert(false);
        return false;
    }   // end if
    fcont->getViewer()->setCamera( focus, nvec, uvec, 500.0f);
    fcont->getViewer()->updateRender();
    return true;
}   // end operator()


bool ActionOrientCameraToFace::isActionable( FaceControl* fcont) const
{
    return FaceTools::hasReqLandmarks( fcont->getModel()->getObjectMeta());
}   // end isActionable

