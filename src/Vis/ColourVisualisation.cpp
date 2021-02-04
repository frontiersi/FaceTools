/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <Vis/ColourVisualisation.h>
#include <FaceModelViewer.h>
#include <r3dvis/VtkTools.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
using FaceTools::Vis::ColourVisualisation;
using FaceTools::Vis::FV;


ColourVisualisation::ColourVisualisation( const QString &lb, float minv, float maxv, float ss, size_t ns)
    : _label(lb), _minr(minv), _maxr(maxv), _minv(minv), _maxv(maxv), _ssize(ss), _nssize(ns)
{
    if ( ss <= 0.0f)
        _ssize = (_maxv - _minv)/20;
    setVisibleRange( minv, maxv);
    _cmapper.setMinColour( Qt::blue);
    _cmapper.setMaxColour( Qt::red);
    _cmapper.setNumColours( 99);
    _cmapper.rebuild();
}   // end ctor


void ColourVisualisation::setVisibleRange( float vmin, float vmax)
{
    _cmapper.setVisibleRange( vmin, vmax);
    _minv = _cmapper.minVisible();
    _maxv = _cmapper.maxVisible();
}   // end setVisibleRange


void ColourVisualisation::rebuildColourMapping()
{
    _cmapper.rebuild();
    for ( FV *fv : _visible)
        fv->refreshColourMap();
}   // end rebuildColourMapping


void ColourVisualisation::setVisible( FV *fv, bool v)
{
    fv->setActiveColours( v ? this : nullptr);  // fv calls (de)activate
}   // end setVisible


void ColourVisualisation::deactivate( FV *fv)
{
    _visible.erase(fv);
    hide( fv);
}   // end deactivate


void ColourVisualisation::activate( FV *fv)
{
    _visible.insert(fv);
    show( fv);
}   // end activate


namespace {
vtkCellData* cellData( vtkActor *a) { return r3dvis::getPolyData(a)->GetCellData();}
vtkPointData* pointData( vtkActor *a) { return r3dvis::getPolyData(a)->GetPointData();}
}   // end namespace


void ColourVisualisation::addCellsArray( vtkActor *a, vtkFloatArray *arr) { cellData(a)->AddArray(arr);}
void ColourVisualisation::addPointsArray( vtkActor *a, vtkFloatArray *arr) { pointData(a)->AddArray(arr);}
void ColourVisualisation::setActiveCellScalars( vtkActor *a, const char *n) { cellData(a)->SetActiveScalars(n);}
void ColourVisualisation::setActiveCellVectors( vtkActor *a, const char *n) { cellData(a)->SetActiveVectors(n);}
void ColourVisualisation::setActivePointScalars( vtkActor *a, const char *n) { pointData(a)->SetActiveScalars(n);}
void ColourVisualisation::setActivePointVectors( vtkActor *a, const char *n) { pointData(a)->SetActiveVectors(n);}
