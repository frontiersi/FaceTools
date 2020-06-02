/************************************************************************
 * Copyright (C) 2020 SIS Research Ltd & Richard Palmer
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

#ifndef FACE_TOOLS_DETECT_FACE_ALIGNMENT_FINDER_H
#define FACE_TOOLS_DETECT_FACE_ALIGNMENT_FINDER_H

#include <FaceTools.h>
#include <r3dvis/OffscreenMeshViewer.h>

namespace FaceTools { namespace Detect {

class FaceTools_EXPORT FaceAlignmentFinder
{
public:
    static bool isInit();

    // Detection uses the Viola and Jones HaarCascades detector to detect 2D features.
    // Provide the initial detection range with d used to scale how closely the camera
    // is positioned to the face based on the detected distance between the eyes e
    // with the distance formula rng * e/d.
    FaceAlignmentFinder( const r3d::KDTree&, float rng, float d=0.30f);

    // Provide the centre point of the model to focus on initially.
    // Returns the matrix specifying how the model IS transformed from its "correct"
    // detected orientation based on the detection of the eyes and the calculation of
    // vertex position eigen vectors around the eyes to estimate facial orientation.
    //
    // This detected alignment is in addition to the model's current transform.
    //
    // If the zero matrix is returned, call error() to return the error msg.
    // Note that the transformation point is set as the midpoint between
    // the detected eyes (which is almost certainly behind the surface of
    // the face. Call eyesSquareRadius to return the square of the radius
    // from this midpoint to either eye.
    r3d::Mat4f find( const Vec3f &focus);

    // Returns distance between the detected eye points.
    inline float interEyeDist() const { return _interEyeDist;}

    inline const std::string& error() const { return _err;}

private:
    r3dvis::OffscreenMeshViewer _vwr;
    const r3d::KDTree &_kdt;
    float _orng, _dfact;
    float _interEyeDist;
    std::string _err;

    float _findEyes( r3d::Vec3f&, r3d::Vec3f&);
};  // end class

}}   // end namespace

#endif
