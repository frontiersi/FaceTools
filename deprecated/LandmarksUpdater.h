/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_DETECT_LANDMARKS_UPDATER_H
#define FACE_TOOLS_DETECT_LANDMARKS_UPDATER_H

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/serialize.h>
#include <dlib/opencv.h>

#include <r3dvis/OffscreenMeshViewer.h>
#include <FaceTools/LndMrk/LandmarkSet.h>

namespace FaceTools {  namespace Detect {

class FaceTools_EXPORT LandmarksUpdater
{
public:
    // Try to initialise the detector from given file.
    static bool initialise( const std::string& faceShapeLandmarksDat);
    static bool isinit();   // Returns true iff initialised.

    LandmarksUpdater( const r3dvis::OffscreenMeshViewer&, const FM*);

    // Sets the basis vectors for local estimation of orientation.
    void setOrientation( const Vec3f& nvec, const Vec3f &uvec);

    // Update the given landmarks - only the ones in uset are modified or are added if they don't already exist.
    // Returns true if landmarks were found to be updated.
    bool detect( Landmark::LandmarkSet&, const IntSet &uset) const;

private:
    const r3dvis::OffscreenMeshViewer &_vwr;
    const FM *_fm;
    Vec3f _xvec, _yvec, _zvec;

    Vec3f _toLocal( float, float, float) const;
    Vec3f _nosetipBase( const Landmark::LandmarkSet&) const;
    bool _setBilateral( const QString&, const Vec3f&, const Vec3f&, Landmark::LandmarkSet&, const IntSet&) const;
    bool _setSingle( const QString&, const Vec3f&, Landmark::LandmarkSet&, const IntSet&) const;

    bool updatePS(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updatePI(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateEN(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateEX(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateP(    const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateMSO(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updatePRN(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSE(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateMND(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateG(    const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSE2(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateMND2( const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateN(    const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateMF(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateCPH(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSN(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateAC(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateAL(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSBAL( const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateCH(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSTS(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSTI(  const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateLS(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updatePG(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateLI(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    bool updateSL(   const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;

    void _setLandmarks( const std::vector<Vec3f>&, Landmark::LandmarkSet&, const IntSet&) const;
    LandmarksUpdater( const LandmarksUpdater&) = delete;
    void operator=( const LandmarksUpdater&) = delete;

    static dlib::shape_predictor s_shapePredictor;
};  // end class

}}   // end namespace

#endif
