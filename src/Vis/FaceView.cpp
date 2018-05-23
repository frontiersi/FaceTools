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
#include <FaceModel.h>
#include <FaceControl.h>
#include <ModelViewer.h>
#include <BaseVisualisation.h>
#include <vtkProperty.h>
#include <VtkTools.h>   // RVTK::transform
#include <iostream>
#include <cassert>
using FaceTools::Vis::FaceView;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using RFeatures::ObjModel;


// public
FaceView::FaceView( const FaceControl* fc) : _fc(fc), _viewer(NULL), _visx(NULL)
{
}   // end ctor


// public
FaceView::~FaceView()
{
    remove();
    _sactor = _tactor = NULL;
}   // end dtor


// public
void FaceView::setViewer( ModelViewer* viewer)
{
    if ( _viewer)
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
    _viewer = viewer;
    if ( _viewer)
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->addActors(_fc);}); // Re-add
}   // end setViewer


// public
void FaceView::rebuild()
{
    // Remove all actors first before building new ones
    if ( _viewer)
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
    std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->purge(_fc);});

    const ObjModel* model = _fc->data()->cmodel();
    RVTK::VtkActorCreator ac;
    _fmap.clear();  // Create the surface actor
    ac.setObjToVTKUniqueFaceMap( &_fmap);
    _sactor = ac.generateSurfaceActor( model);

    _tactor = NULL;
    if ( model->getNumMaterials() == 1) // Create the textured actor
    {
        std::vector<vtkSmartPointer<vtkActor> > tactors;
        ac.generateTexturedActors( model, tactors);
        assert(tactors.size() == 1);
        _tactor = tactors[0];
    }   // end else
    else
    {
        std::cerr << "[ERROR] FaceTools::Vis::FaceView::rebuild: Cannot build textured vtkActor; ";
        if ( model->getNumMaterials() == 0)
            std::cerr << "No texture maps found on provided model!";
        else
            std::cerr << "ObjModel has more than one material - merge them first!";
        std::cerr << std::endl;
    }   // end else

    std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->apply(_fc);});     // Re-apply
    if ( _viewer)   // Re-add if a viewer set
    {
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->addActors(_fc);}); // Re-add
        _viewer->updateRender();
    }   // end if
}   // end rebuild


// public
void FaceView::remove( BaseVisualisation* vis)
{
    if ( !vis)  // Remove all visualisations?
    {
        if ( _viewer)
            std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
        _vlayers.clear();
        _visx = NULL;
    }   // end if
    else if ( _vlayers.count(vis) > 0)
    {
        if ( _viewer)
            vis->removeActors(_fc);
        _vlayers.erase(vis);
        if ( _visx == vis)
            _visx = NULL;
    }   // end else
}   // end remove


// public
bool FaceView::apply( BaseVisualisation* vis)
{
    if ( !_sactor)
    {
        std::cerr << "[ERROR] FaceTools::Vis::FaceView::apply: "
                  << "No surface vtkActor built! Must call rebuild() before apply()!" << std::endl;
        return false;
    }   // end if

    if ( !_viewer)
    {
        std::cerr << "[WARNING] FaceTools::Vis::FaceView::apply: "
                  << "Cannot apply visualisations without a viewer set!" << std::endl;
        return false;
    }   // end if

    if ( !vis)  // Refresh existing
    {
        if (_vlayers.empty())
        {
            std::cerr << "[ERROR] FaceTools::Vis::FaceView::apply: "
                      << "Cannot refresh visualisations when none yet set!" << std::endl;
            return false;
        }   // end if

        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->apply(_fc);});     // Re-apply
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->addActors(_fc);}); // Re-add
    }   // end if
    else
    {
        // Note that we first text if we're adding a new visualisation and if this will replace the existing
        // exclusive visualisation, because we want to remove the actor(s) before applying the new visualisation.
        // This ought not to make a difference but control over the removal of actors is delegated to some other
        // BaseVvisualisation derived type which may implement strange (incorrect) logic expecting the visualisation
        // properties of the actor to be the same upon removal as when added (because it labels itself as an
        // exclusive visualisation), so this ordering of removal first before applying is done to avoid this
        // potential issue.
        if ( _vlayers.count(vis) == 0)
        {
            // If this is an exclusive visualisation, it needs to replace the old exclusive one.
            if ( vis->isExclusive())
            {
                if ( _visx)
                {
                    _visx->removeActors(_fc);
                    _vlayers.erase(_visx);
                }   // end if
                _visx = vis;
            }   // end if
            _vlayers.insert(vis);

            vis->apply(_fc);   // A - see note above about this line and line B
            vis->addActors(_fc);
        }   // end if
        else
            vis->apply(_fc);   // B - see note above about this line and line A
    }   // end else

    return true;
}   // end apply


// public
const BaseVisualisation* FaceView::belongs( const vtkProp* prop) const
{
    if ( !prop)
        return NULL;
    if ( _sactor == prop || _tactor == prop)
        return _visx;
    for ( const BaseVisualisation* vis : _vlayers)    // Test all the visualisation layers
    {
        if ( vis->belongs( prop, _fc))
            return vis;
    }   // end for
    return NULL;
}   // end belongs


// public
double FaceView::opacity() const { return _sactor ? _sactor->GetProperty()->GetOpacity() : 0.0;}

// public
void FaceView::setOpacity( double v)
{
    if ( _sactor)
        _sactor->GetProperty()->SetOpacity(v);
    if ( _tactor)
        _tactor->GetProperty()->SetOpacity(v);
}   // end setOpacity


// public
QColor FaceView::colour() const
{
    QColor c;
    if ( _sactor)
    {
        double* vc = _sactor->GetProperty()->GetColor();
        c = QColor::fromRgbF( vc[0], vc[1], vc[2]);
    }   // end if
    return c;
}   // end colour


void FaceView::setColour( const QColor& c)
{
    if ( _sactor)
        _sactor->GetProperty()->SetColor( c.redF(), c.greenF(), c.blueF());
}   // end setColour


bool FaceView::backfaceCulling() const
{
    bool bf = false;
    if ( _sactor)
        bf = _sactor->GetProperty()->GetBackfaceCulling();
    return bf;
}   // end backfaceCulling


void FaceView::setBackfaceCulling( bool v)
{
    if ( _sactor)
        _sactor->GetProperty()->SetBackfaceCulling( v);
    if ( _tactor)
        _tactor->GetProperty()->SetBackfaceCulling( v);
}   // end setBackfaceCulling


// public
vtkSmartPointer<vtkMatrix4x4> FaceView::userTransform() const
{
    vtkSmartPointer<vtkMatrix4x4> umat = vtkSmartPointer<vtkMatrix4x4>::New();
    umat->Identity();   // Make identity
    if ( _sactor)  // Return identity matrix if no actors built yet
    {
        // Find out which actor has been moved and apply its transformation to the other actor.
        if ( !_sactor->GetIsIdentity())
            umat = _sactor->GetMatrix();
        else if ( _tactor && !_tactor->GetIsIdentity())
            umat = _tactor->GetMatrix();
    }   // end if
    return umat;
}   // end userTransform


void FaceView::transform( const vtkMatrix4x4* t)
{
    assert(t);
    if ( !_sactor)
        return;

    std::cerr << " Transforming visualisation layers with matrix:" << std::endl;
    std::cerr << " " << RVTK::toCV(t) << std::endl;

    RVTK::transform(_sactor, t);
    if ( _tactor)
        RVTK::transform(_tactor, t);
    // Apply the transform to applied visualisation layers.
    std::for_each( std::begin(_vlayers), std::end(_vlayers), [=](auto v){ v->transform( _fc, t);});
    if ( _viewer)
        _viewer->updateRender();
}   // end transform
