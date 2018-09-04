## FaceTools

The FaceTools library provides the core functionality for the [Cliniface](../../../cliniface/) application.

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

### Copyright notice

The FaceTools library is copyright Spatial Information Systems Research Limited, 2018.  

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
