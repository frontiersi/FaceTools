/************************************************************************
 * Copyright (C) 2019 SIS Research Ltd & Richard Palmer
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

#include <Vis/ScalarVisualisation.h>
#include <FaceModelViewer.h>
#include <cassert>
using FaceTools::Vis::ScalarVisualisation;
using FaceTools::Vis::SurfaceVisualisation;
using FaceTools::Vis::FV;
using FaceTools::FM;


ScalarVisualisation::ScalarVisualisation( const std::string& lb, bool mp, float minv, float maxv, float ss, size_t ns)
    : SurfaceVisualisation<ScalarVisualisation>( QString::fromStdString(lb), mp),
      _minr(minv), _maxr(maxv), _minv(minv), _maxv(maxv), _ssize(ss), _nssize(ns)
{
    if ( ss <= 0.0f)
        _ssize = (_maxv - _minv)/20;
    setVisibleRange( minv, maxv);
    _cmapper.setMinColour( QColor(0  ,   0, 255));   // Blue
    _cmapper.setMaxColour( QColor(255,   0,   0));   // Red
    _cmapper.setNumColours( 99);
    _cmapper.rebuild();
}   // end ctor

ScalarVisualisation::~ScalarVisualisation() {}

void ScalarVisualisation::setVisibleRange( float vmin, float vmax)
{
    _cmapper.setVisibleRange(vmin,vmax);
    _minv = _cmapper.minVisible();
    _maxv = _cmapper.maxVisible();
}   // end setVisibleRange


void ScalarVisualisation::setNumColours( size_t v) { _cmapper.setNumColours( v);}
void ScalarVisualisation::setMinColour( const QColor& c) { _cmapper.setMinColour(c);}
void ScalarVisualisation::setMaxColour( const QColor& c) { _cmapper.setMaxColour(c);}

size_t ScalarVisualisation::numColours() const { return _cmapper.numColours();}
QColor ScalarVisualisation::minColour() const { return _cmapper.minColour();}
QColor ScalarVisualisation::maxColour() const { return _cmapper.maxColour();}

vtkLookupTable* ScalarVisualisation::lookupTable( const vtkRenderer* ren)
{
    return _cmapper.lookupTable( ren);
}   // end lookupTable


void ScalarVisualisation::rebuild()
{
    _cmapper.rebuild();
    for ( FV *fv : _activated)
    {
        vtkActor *actor = const_cast<vtkActor*>( fv->actor());
        actor->GetMapper()->SetScalarRange( minVisible(), maxVisible());
        fv->viewer()->updateRender();   // Force through the render update (necessary!)
    }   // end for
}   // end rebuild


void ScalarVisualisation::setVisible( FV *fv, bool v)
{
    if ( v)
        fv->setActiveScalars( this);    // Through this call, FaceView will call this->activate
    else if ( fv->activeScalars() == this)
        fv->setActiveScalars( nullptr);
}   // end setVisible


bool ScalarVisualisation::isVisible( const FV *fv) const
{
    return fv->activeScalars() == this;
}   // end isVisible


void ScalarVisualisation::deactivate( FV *fv)
{
    vtkActor *actor = const_cast<vtkActor*>( fv->actor());
    const std::string lab = label().toStdString();

    // NB While both vtkCellData and vtkPointData are vtkDataSetAttributes, VTK's design
    // precludes being able to use vtkDataSetAttributes::SetActiveScalars polymorphically.
    if ( mapsPolys())
        r3dvis::getPolyData(actor)->GetCellData()->SetActiveScalars("");
    else
        r3dvis::getPolyData(actor)->GetPointData()->SetActiveScalars("");
    actor->GetMapper()->SetScalarVisibility( false);
    _activated.erase(fv);
}   // end deactivate


void ScalarVisualisation::activate( FV *fv)
{
    vtkActor *actor = const_cast<vtkActor*>(fv->actor());
    vtkPolyData *pd = r3dvis::getPolyData( actor);
    const std::string lab = label().toStdString();

    actor->GetMapper()->SetLookupTable( lookupTable( fv->viewer()->getRenderer()));

    // NB While both vtkCellData and vtkPointData are vtkDataSetAttributes, VTK's design
    // precludes being able to use vtkDataSetAttributes::SetActiveScalars polymorphically.
    if ( mapsPolys())
        pd->GetCellData()->SetActiveScalars( lab.c_str());
    else
        pd->GetPointData()->SetActiveScalars( lab.c_str());

    actor->GetMapper()->SetScalarRange( minVisible(), maxVisible());
    actor->GetMapper()->SetScalarVisibility( true);
    _activated.insert(fv);
}   // end activate
