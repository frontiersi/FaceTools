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

#include <Vis/VectorVisualisation.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Vis::VectorVisualisation;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;


VectorVisualisation::VectorVisualisation( const std::string& lb, bool mp)
    : SurfaceVisualisation<VectorVisualisation>( QString::fromStdString(lb), mp) {}


void VectorVisualisation::setVisible( FV *fv, bool v)
{
    if ( v)
        fv->setActiveVectors( this);    // Through this call, FaceView will call this->activate
    else if ( fv->activeVectors() == this)
        fv->setActiveVectors( nullptr);
}   // end setVisible


bool VectorVisualisation::isVisible( const FV *fv) const
{
    return fv->activeVectors() == this;
}   // end isVisible


void VectorVisualisation::setScalarMapping( const FV *fv, ScalarVisualisation *svis)
{
    assert( _glyphs.count(fv) > 0);
    r3dvis::VtkVectorMap::Ptr vmap = _glyphs.at(fv);
    if ( !svis)
    {
        vmap->setColour( 0, 0, 1.0);
        vmap->setScaleFactor( 5.0);
    }   // end if
    else
    {
        const float minVis = svis->minVisible();
        const float maxVis = svis->maxVisible();
        vmap->setScalarColourLookup( svis->lookupTable( fv->viewer()->getRenderer()), minVis, maxVis);
        vmap->setScaleFactor( 5.0 / fabsf(maxVis));
    }   // end else
}   // end setScalarMapping


void VectorVisualisation::deactivate( FV *fv)
{
    assert( _glyphs.count(fv) > 0);
    // NB While both vtkCellData and vtkPointData are vtkDataSetAttributes, VTK's design
    // precludes being able to use vtkDataSetAttributes::SetActiveScalars polymorphically.
    if ( mapsPolys())
        r3dvis::getPolyData(fv->actor())->GetCellData()->SetActiveVectors("");
    else
        r3dvis::getPolyData(fv->actor())->GetPointData()->SetActiveVectors("");
    fv->viewer()->remove( _glyphs[fv]->prop());
    _glyphs.erase(fv);
}   // end deactivate


void VectorVisualisation::activate( FV *fv)
{
    assert( _glyphs.count(fv) == 0);
    vtkPolyData *pd = r3dvis::getPolyData( fv->actor());
    const std::string lab = label().toStdString();

    // NB While both vtkCellData and vtkPointData are vtkDataSetAttributes, VTK's design
    // precludes being able to use vtkDataSetAttributes::SetActiveScalars polymorphically.
    if ( mapsPolys())
        pd->GetCellData()->SetActiveVectors( lab.c_str());
    else
        pd->GetPointData()->SetActiveVectors( lab.c_str());

    r3dvis::VtkVectorMap::Ptr vmap = r3dvis::VtkVectorMap::create( pd, false);
    vmap->setColour( 1.0, 1.0, 1.0);
    vmap->setOpacity( 0.99);
    vmap->setPickable(false);
    fv->viewer()->add( vmap->prop());
    _glyphs[fv] = vmap;
}   // end activate


void VectorVisualisation::syncWithViewTransform( const FV *fv)
{
    if ( _glyphs.count(fv) > 0)
        _glyphs.at(fv)->pokeTransform( fv->transformMatrix());
}   // end syncWithViewTransform
