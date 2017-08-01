#include <SurfaceMesher.h>
using FaceTools::SurfaceMesher;
using RFeatures::ObjModel;
#include <cassert>
#include <algorithm>
#include <boost/foreach.hpp>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Advancing_front_surface_reconstruction.h>

namespace _SMESH_
{

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
        BOOST_FOREACH ( int vidx, vidxs)
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
        model->setFace( _points2ModelVidxs.at(f[0]),
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

    using _SMESH_::MeshConstructor;
    MeshConstructor meshConstructor(_model);
    meshConstructor.mesh();

    return _model->getNumFaces();
}   // end operator()

