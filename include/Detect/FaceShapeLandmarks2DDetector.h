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

#ifndef FACE_TOOLS_DETECT_FACE_SHAPE_LANDMARKS_2D_DETECTOR_H
#define FACE_TOOLS_DETECT_FACE_SHAPE_LANDMARKS_2D_DETECTOR_H

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/serialize.h>
#include <dlib/opencv.h>

#include <OffscreenModelViewer.h>
#include <LandmarkSet.h>

namespace FaceTools {  namespace Detect {

class FaceTools_EXPORT FaceShapeLandmarks2DDetector
{
public:
    // Try to initialise the detector from given file.
    static bool initialise( const std::string& faceShapeLandmarksDat);
    static bool isinit();   // Returns true iff initialised.

    static bool detect( const RVTK::OffscreenModelViewer&,
                        const FM*,
                        Landmark::LandmarkSet&, // The landmarks to update.
                        const IntSet& ulmks);   // Only landmark IDs in this set will be updated.

private:
    static dlib::shape_predictor s_shapePredictor;
};  // end class

}}   // end namespace

#endif
