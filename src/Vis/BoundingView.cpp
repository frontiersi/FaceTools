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

#include <Vis/BoundingView.h>
#include <vtkProperty.h>
#include <FaceModel.h>
#include <vtkPolyDataMapper.h>
using FaceTools::Vis::BoundingView;
using FaceTools::Vis::SimpleView;
using FaceTools::FM;


BoundingView::BoundingView( const FM* fm, float lw) : SimpleView(), _fm(fm)
{
    assert( fm);
    assert( !fm->bounds().empty());
    const RFeatures::ObjModelBounds& bounds = *fm->bounds()[0];
    // Get the untransformed corners to create an upright cuboid
    const cv::Vec6d cb = bounds.cornersAs6d();
    _cubeSource->SetBounds( &cb[0]);

    vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputConnection( _cubeSource->GetOutputPort());
    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);

    actor->SetPickable(false);
    vtkProperty* prop = actor->GetProperty();
    prop->SetRepresentationToWireframe();
    prop->SetRenderLinesAsTubes(false);
    prop->SetLineWidth( lw);

    initActor( actor);
    pokeTransform( actor, RVTK::toVTK( fm->model().transformMatrix()));
}   // end ctor


void BoundingView::pokeTransform( const vtkMatrix4x4* d) { SimpleView::pokeTransform(d);}


void BoundingView::pokeTransform( vtkActor* actor, const vtkMatrix4x4* d)
{
    /*
    static int fcount = 0;
    std::cerr << "\nFCOUNT: " << (fcount++) << std::endl;
    */

    // Bounds transform already has model transform applied so need to remove before adding the provided view transform.
    const cv::Matx44d& mT = _fm->model().transformMatrix();
    //std::cerr << "Model transform matrix:" << std::endl;
    //std::cerr << mT << std::endl;

    assert( !_fm->bounds().empty());
    const RFeatures::ObjModelBounds& bounds = *_fm->bounds()[0];
    const cv::Matx44d& bT = bounds.transformMatrix();
    //std::cerr << "Bounds transform matrix:" << std::endl;
    //std::cerr << bT << std::endl;

    const cv::Matx44d vT = RVTK::toCV( d);
    //std::cerr << "View transform matrix:" << std::endl;
    //std::cerr << vT << std::endl;

    // Remove the model transform from the bounds transform. During dynamic pokes, the model transform is I so this
    // operation will leave the bounds transform intact. During static pokes we need to remove the model transform
    // (which will not be I) otherwise we will be double counting the transform when applying the bounds transform
    // before the view transform.
    const cv::Matx44d sT = mT.inv() * bT;
    //std::cerr << "Static transform matrix:" << std::endl;
    //std::cerr << sT << std::endl;

    vtkSmartPointer<vtkMatrix4x4> pmat = RVTK::toVTK( vT * sT);

    actor->PokeMatrix( pmat);
}   // end pokeTransform
