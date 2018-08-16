# FaceTools

## Prerequisites
- [CGAL](http://www.cgal.org/)
- [dlib](http://dlib.net/)
- [QuaZip](https://github.com/stachenov/quazip)
- [rlib](https://github.com/richeytastic/rlib)
- [rFeatures](https://github.com/richeytastic/rfeatures)
- [rVTK](https://github.com/richeytastic/rvtk)
- [rModelIO](https://github.com/richeytastic/rmodelio)
- [QTools](https://github.com/richeytastic/qtools)
- [libbuild](https://github.com/richeytastic/libbuild)

dlib requires the `shape_predictor_68_face_landmark.dat`. Download from:
<http://dlib.net/files/shape_predictor_68_face_landmarks.dat.bz2>

The version of dlib tested (19.4) contains a buggy `CMakeLists.txt` in the dlib directory.
Replace with `dlib_CMakeLists.txt` contained in this repository before building.
The bug prevents building of the dlib dll in Windows (not a problem in Linux).

A version of QuaZip is included here (custom_quazip.zip) which uses customised CMake
scripts that leverage the build scripts of [libbuild](https://github.com/richeytastic/libbuild)
so that QuaZip can be easily found when building FaceTools.
