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

#include <FaceView.h>
#include <ModelViewer.h>
#include <VisualisationAction.h>
#include <VtkActorCreator.h>    // RVTK
#include <vtkProperty.h>
#include <cassert>
using FaceTools::FaceView;
using FaceTools::ModelViewer;
using FaceTools::VisualisationAction;
using RFeatures::ObjModel;


// public
FaceView::FaceView( const ObjModel::Ptr model)
    : _viewer( NULL), _isshown(false), _istexture(false)
{
    reset(model);
}   // end ctor


// public
FaceView::~FaceView()
{
    setVisible(false);
}   // end dtor


// public
void FaceView::setVisible( bool visible, ModelViewer* viewer)
{
    if ( _viewer)
    {
        _viewer->remove(_tactor);
        _viewer->remove(_sactor);
    }   // end if

    if ( viewer)
    {
        viewer->remove(_tactor);
        viewer->remove(_sactor);
    }   // end else

    _isshown = false;
    _viewer = viewer;
    if ( visible && viewer)
    {
        viewer->add( getActor());
        _isshown = true;
    }   // end if
}   // end setVisible


// public
bool FaceView::isVisible() const { return _isshown;}
vtkActor* FaceView::getActor() const { return isTexture() ? _tactor : _sactor;}
vtkActor* FaceView::getSurfaceActor() const { return _sactor;}
void FaceView::setTexture( bool enable) { _istexture = enable && _tactor != NULL;}
bool FaceView::isTexture() const { return _istexture;}
const boost::unordered_map<int,int>* FaceView::getPolyIdLookups() const { return &_flookup;}


// public
void FaceView::setOptions( const FaceTools::ModelOptions& vo)
{
    _opts = vo;
    _sactor->GetProperty()->SetPointSize( _opts.model.vertexSize);
    _sactor->GetProperty()->SetLineWidth( _opts.model.lineWidth);
    const QColor& mscol = _opts.model.surfaceColourMid;
    _sactor->GetProperty()->SetColor( mscol.redF(), mscol.greenF(), mscol.blueF());
    _sactor->GetProperty()->SetOpacity( mscol.alphaF());
    // Backface culling
    _sactor->GetProperty()->SetBackfaceCulling( _opts.model.backfaceCulling);
    if ( _tactor)
        _tactor->GetProperty()->SetBackfaceCulling( _opts.model.backfaceCulling);
}   // end setOptions


// public
void FaceView::reset( const ObjModel::Ptr model)
{
    const bool shown = isVisible();
    bool istex = isTexture();
    setVisible(false);
    setTexture(false);

    // Create the actors from the current data
    RVTK::VtkActorCreator ac;
    ac.setObjToVTKUniqueFaceMap( &_flookup);
    _sactor = ac.generateSurfaceActor( model);

    // Create the textured actor if materials available
    _tactor = NULL;
    if ( model->getNumMaterials() > 0)
    {
        std::vector<vtkSmartPointer<vtkActor> > tactors;
        //RVTK::VtkActorCreator ac2;
        //ac2.generateTexturedActors( model, tactors);
        ac.generateTexturedActors( model, tactors);
        assert(tactors.size() == 1);
        _tactor = tactors[0];
    }   // end if

    setOptions( _opts);
    setTexture(istex && _tactor);
    setVisible(shown);
}   // end reset
