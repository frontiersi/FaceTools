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

#include <SurfaceMesher.h>
using FaceTools::SurfaceMesher;
using RFeatures::ObjModel;
#include <cassert>
#include <algorithm>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>

namespace {

typedef CGAL::Exact_predicates_inexact_constructions_kernel KRN;
typedef CGAL::cpp11::array<std::size_t,3> Facet;


struct MeshConstructor
{
    ObjModel::Ptr model;
    
    explicit MeshConstructor( ObjModel::Ptr m) : model(m)
    {
        size_t i = 0;
        const IntSet& vidxs = model->getVertexIds();
        _points.resize( vidxs.size());
        for ( int vidx : vidxs)
        {
            const cv::Vec3f& v = model->vtx(vidx);
            _points[i] = KRN::Point_3( v[0], v[1], v[2]);
            _points2ModelVidxs[i++] = vidx;
        }   // end foreach
    }   // end ctor

    void mesh()
    {
        CGAL::advancing_front_surface_reconstruction( _points.begin(), _points.end(), *this);
    }   // end mesh

    MeshConstructor& operator=( const Facet f)
    {
        model->addFace( _points2ModelVidxs.at(f[0]),
                        _points2ModelVidxs.at(f[1]),
                        _points2ModelVidxs.at(f[2]));
        return *this;
    }   // end operator=

    MeshConstructor& operator*() { return *this; }
    MeshConstructor& operator++() { return *this; }
    MeshConstructor operator++( int) { return *this; }

private:
    std::vector<KRN::Point_3> _points;
    boost::unordered_map<size_t,int> _points2ModelVidxs;
};  // end struct

}   // end namespace


// public
SurfaceMesher::SurfaceMesher( ObjModel::Ptr m) : _model(m) {}


// public
int SurfaceMesher::operator()()
{
    if ( _model->getNumFaces() > 0)
        return -1;
    if ( _model->getNumVertices() < 3)
        return -2;

    MeshConstructor meshConstructor(_model);
    meshConstructor.mesh();

    return (int)_model->getNumFaces();
}   // end operator()

