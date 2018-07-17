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
#include <QColor>
#include <iostream>
#include <cassert>
using FaceTools::Vis::FaceView;
using FaceTools::Vis::BaseVisualisation;
using FaceTools::ModelViewer;
using FaceTools::FaceControl;
using RFeatures::ObjModel;


// public
FaceView::FaceView( const FaceControl* fc) : _fc(fc), _viewer(nullptr), _visx(nullptr)
{
}   // end ctor


// public
FaceView::~FaceView()
{
    remove();
    _sactor = _tactor = nullptr;
}   // end dtor


// public
void FaceView::setViewer( ModelViewer* viewer)
{
    if ( _viewer)
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
    _viewer = viewer;
    if ( _viewer)
    {
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->apply(_fc);});     // Re-apply
        std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->addActors(_fc);}); // Re-add
    }   // end if
}   // end setViewer


// public
void FaceView::reset()
{
    std::cerr << "[INFO] FaceTools::Vis::FaceView::reset: Resetting view models" << std::endl;
    std::unordered_set<BaseVisualisation*> vlayers = _vlayers;  // Copy out
    remove();   // Remove all

    const FaceModel* fm = _fc->data();
    //fm->lockForRead();

    const ObjModel* model = fm->info()->cmodel();
    RVTK::VtkActorCreator ac;
    _fmap.clear();  // Create the surface actor
    ac.setObjToVTKUniqueFaceMap( &_fmap);

    _sactor = ac.generateSurfaceActor( model);
    _tactor = nullptr;
    if ( model->getNumMaterials() == 1) // Create the textured actor
    {
        std::vector<vtkActor*> tactors;
        ac.generateTexturedActors( model, tactors);
        assert(tactors.size() == 1);
        _tactor = tactors[0];
    }   // end else
    else
    {
        std::cerr << "[WARNING] FaceTools::Vis::FaceView::reset: Cannot build textured vtkActor; ";
        if ( model->getNumMaterials() == 0)
            std::cerr << "No texture maps found on provided model!";
        else
            std::cerr << "ObjModel has more than one material - merge them first!";
        std::cerr << std::endl;
    }   // end else

    //fm->unlock();
    for ( BaseVisualisation* vis : vlayers)
        apply(vis);
}   // end reset


// public
void FaceView::remove( BaseVisualisation* vis)
{
    if ( !vis)  // Remove all visualisations?
    {
        if ( _viewer)
        {
            std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->removeActors(_fc);});
            std::for_each( std::begin(_vlayers), std::end(_vlayers), [this](auto v){v->purge(_fc);});
        }   // end if
        _vlayers.clear();
        _visx = nullptr;
    }   // end if
    else if ( _vlayers.count(vis) > 0)
    {
        if ( _viewer)
        {
            vis->removeActors(_fc);
            vis->purge(_fc);
        }   // end if
        _vlayers.erase(vis);
        if ( _visx == vis)
            _visx = nullptr;
    }   // end else
}   // end remove


// public
bool FaceView::apply( BaseVisualisation* vis)
{
    assert(_sactor);
    if ( !_sactor)
    {
        std::cerr << "[ERROR] FaceTools::Vis::FaceView::apply: "
                  << "No surface vtkActor built! Must call reset() before apply()!" << std::endl;
        return false;
    }   // end if

    assert(_viewer);
    if ( !_viewer)
    {
        std::cerr << "[WARNING] FaceTools::Vis::FaceView::apply: "
                  << "Cannot apply visualisations without a viewer set!" << std::endl;
        return false;
    }   // end if

    if ( !vis)
    {
        auto vlayers = _vlayers;    // Copy out
        std::for_each( std::begin(vlayers), std::end(vlayers), [this](auto v){this->apply(v);});
        return true;
    }   // end if

    assert( vis->isAvailable(_fc->data()));
    if ( !vis->isAvailable(_fc->data()))
    {
        std::cerr << "[WARNING] FaceTools::Vis::FaceView::apply: "
                  << "Cannot apply visualisations - is not available for the FaceModel!" << std::endl;
        return false;
    }   // end if

    remove(vis);

    // First test if adding a new visualisation and if this will replace the existing exclusive visualisation,
    // because we want to remove the actor(s) before applying the new visualisation. This ought not to make a
    // difference but control over the removal of actors is delegated to some other BaseVisualisation derived
    // type which may implement strange (incorrect) logic expecting the visualisation properties of the actor
    // to be the same upon removal as when added (because it labels itself as exclusive). This ordering of
    // removal first before applying is done to avoid this potential issue.
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

    return true;
}   // end apply


// public
bool FaceView::isApplied( const BaseVisualisation *vis) const { return _vlayers.count(const_cast<BaseVisualisation*>(vis)) > 0;}


// public
BaseVisualisation* FaceView::belongs( const vtkProp* prop) const
{
    if ( !prop)
        return nullptr;
    if ( _sactor == prop || _tactor == prop)
        return _visx;
    for ( BaseVisualisation* vis : _vlayers)    // Test all the visualisation layers
    {
        if ( vis->belongs( prop, _fc))
            return vis;
    }   // end for
    return nullptr;
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
    assert(_sactor);
    vtkSmartPointer<vtkMatrix4x4> umat = vtkMatrix4x4::New();
    umat->Identity();   // Make identity
    // Return the transform matrix for the actor of the exclusive visualisation.
    if ( _visx->belongs(_sactor, _fc))
        _sactor->GetMatrix( umat);
    else if ( _tactor && _visx->belongs(_tactor, _fc))
        _tactor->GetMatrix( umat);
    return umat;
}   // end userTransform


// public
void FaceView::pokeTransform( const vtkMatrix4x4* M, bool transEx)
{
    assert(_sactor);
    assert(_visx);
    if ( transEx || !_visx->belongs( _sactor, _fc))
        _sactor->PokeMatrix( const_cast<vtkMatrix4x4*>(M));
    if ( _tactor && ( transEx || !_visx->belongs( _tactor, _fc)))
        _tactor->PokeMatrix( const_cast<vtkMatrix4x4*>(M));
    std::for_each( std::begin(_vlayers), std::end(_vlayers), [=](auto v){ v->pokeTransform( _fc, M);});
}   // end pokeTransform


// public
void FaceView::fixTransform()
{
    assert(_sactor);
    RVTK::transform( _sactor, _sactor->GetMatrix());
    if ( _tactor)
        RVTK::transform( _tactor, _tactor->GetMatrix());
    std::for_each( std::begin(_vlayers), std::end(_vlayers), [=](auto v){ v->fixTransform( _fc);});
    vtkSmartPointer<vtkMatrix4x4> I = vtkMatrix4x4::New();
    I->Identity();   // Make identity
    pokeTransform( I, true);    // Set the user matrix back to the identity.
}   // end fixTransform
