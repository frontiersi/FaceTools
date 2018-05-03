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

#include <FacePreProcessor.h>
#include <FaceTools.h>
#include <Landmarks.h>
#include <ObjModelVertexAdder.h>    // RFeatures
#include <ObjModelHoleFiller.h>     // RFeatures
#include <cassert>
using FaceTools::FacePreProcessor;
using FaceTools::ObjMetaData;
using RFeatures::ObjModel;


FacePreProcessor::FacePreProcessor( ObjMetaData::Ptr omd) : _omd(omd)
{
    // Can only be calculated if nose tip and eye landmarks present
    const double crad = FaceTools::calcFaceCropRadius( _omd, 1.0);
    if ( crad <= 0)
    {
        std::cerr << "[ERROR] FaceTools::FacePreProcessor::ctor: ObjMetaData missing necessary landmarks!" << std::endl;
        assert(false);
    }   // end if
}   // end ctor


void FacePreProcessor::operator()( bool fillHoles, double mta, double nl, double sfactor)
{
    const cv::Vec3f fc = FaceTools::calcFaceCentre( _omd);  // Based on eyes and nosetip landmarks
    ObjModel::Ptr model = _omd->getObject();          // To modify

    // Crop #1
    if ( nl > 0)
    {
        double crad = FaceTools::calcFaceCropRadius( _omd, nl+0.2);
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Crop #1 within " << crad << " mm of face centre..." << std::endl;
        const int svid = _omd->getKDTree()->find( _omd->landmarks()->getLandmark( FaceTools::Landmarks::NASAL_TIP));
        model = FaceTools::crop( model, fc, crad, svid);
    }   // end if

    // Fill holes
    if ( fillHoles)
    {
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Filling holes... ";
        const int nfilled = RFeatures::ObjModelHoleFiller::fillHoles( model) - 1;
        if ( nfilled > 0)
        {
            std::cerr << nfilled << " filled";
            FaceTools::clean( model);
        }   // end if
        else
            std::cerr << " none found";
        std::cerr << std::endl;
    }   // end if

    // Increase vertex density
    if ( mta > 0)
    {
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Subdividing and merging polys with area > " << mta << " mm^2..." << std::endl;
        RFeatures::ObjModelVertexAdder vadder(model);
        vadder.subdivideAndMerge( mta);
    }   // end if

    // Crop #2 (for smoother boundary)
    if ( nl > 0.0)
    {
        double crad = FaceTools::calcFaceCropRadius( _omd, nl);
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Crop #2 within " << crad << " mm of face centre..." << std::endl;
        model = FaceTools::crop( model, fc, crad, 0);
    }   // end if

    // Smooth
    if ( sfactor > 0)
    {
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Smoothing (" << sfactor << " factor over max 10 iterations)..." << std::endl;
        RFeatures::ObjModelCurvatureMap::Ptr cmap = RFeatures::ObjModelCurvatureMap::create( model, *model->getFaceIds(0).begin());
        size_t numSmoothIterations = 10;
        RFeatures::ObjModelSmoother( cmap).smooth( sfactor, numSmoothIterations);

        const double lmshift = _omd->shiftLandmarksToSurface();    // Map landmarks back to the surface (which changed from the smooth operation).
        std::cerr << "[INFO] FaceTools::FacePreProcessor::preprocess: Shifted landmarks to surface by mean of " << lmshift << " units" << std::endl;
        // Shifting the landmarks slightly will mean the face centre is slightly different, but it will be a small difference that can be ignored.
    }   // end if

    _omd->setObject( model);
}   // end operator()
