# FaceTools

## Prerequisites
- [CGAL](http://www.cgal.org/)
- [dlib](http://dlib.net/)
- [QTools](../../../QTools)

dlib requires the shape_predictor_68_face_landmark.dat. Download from:
http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2

The version of dlib tested (19.4) contains a buggy CMakeLists.txt in the dlib directory.
Replace with dlib_CMakeLists.txt contained in this repository before building.
The bug prevents building of the dlib dll in Windows. This is not a problem in Linux.
