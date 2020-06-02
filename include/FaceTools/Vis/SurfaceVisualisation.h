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

#ifndef FACE_TOOLS_VIS_SURFACE_VISUALISATION_H
#define FACE_TOOLS_VIS_SURFACE_VISUALISATION_H

/**
 * Abstract application of scalars or vectors to an actor's surface polygons.
 * Uses the Curiously Recurring Template Pattern (static polymorphism). Derived
 * types should pass themselves as template parameters to instances of this type
 * (SurfaceVisualisation). ScalarVisualisation and VectorVisualisation are derived.
 */

#include "BaseVisualisation.h"
#include "FaceView.h"
#include <FaceTools/FaceModel.h>
#include <r3dvis/VtkTools.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>

namespace FaceTools { namespace Vis {

template <typename Derived>
class FaceTools_EXPORT SurfaceVisualisation : public BaseVisualisation
{
public:
    SurfaceVisualisation( const QString &label, bool mapsPolys)
        : _label(label), _mapsPolys(mapsPolys) {}

    const char* name() const override { return "SurfaceVisualisation";}

    const QString& label() const { return _label;}

    // Returns true iff this visualisation can be used to map the given data.
    bool isAvailable( const FV *fv, const QPoint*) const override
    {
        assert(fv);
        return static_cast<const Derived*>(this)->isAvailable(fv);
    }   // end isAvailable

    void apply( const FV* fv, const QPoint* mc=nullptr) override
    {
        if ( !_hasMetrics( fv))
            _obtainMetrics( fv);
    }   // end apply

    void setVisible( FV* fv, bool v) override { static_cast<Derived*>(this)->setVisible( fv, v);}
    bool isVisible( const FV* fv) const override { return static_cast<const Derived*>(this)->isVisible( fv);}

    // Allow this class to decide if it's appropriate to purge data for the given event and face view.
    void purge( const FV* fv) override
    {
        const std::string lab = _label.toStdString();
        if ( _mapsPolys)
            r3dvis::getPolyData( fv->actor())->GetCellData()->RemoveArray(lab.c_str());
        else
            r3dvis::getPolyData( fv->actor())->GetPointData()->RemoveArray(lab.c_str());
    }   // end purge

protected:
    inline bool mapsPolys() const { return _mapsPolys;}

    virtual vtkSmartPointer<vtkFloatArray> mapMetrics( const FV*) = 0;

private:
    const QString _label;
    const bool _mapsPolys;

    bool _hasMetrics( const FV *fv) const
    {
        const std::string lab = _label.toStdString();
        vtkPolyData *pd = r3dvis::getPolyData( fv->actor());
        bool hasArray = false;
        if ( _mapsPolys)
            hasArray = pd->GetCellData()->GetAbstractArray( lab.c_str()) != nullptr;
        else 
            hasArray = pd->GetPointData()->GetAbstractArray( lab.c_str()) != nullptr;
        return hasArray;
    }   // end _hasMetrics

    void _obtainMetrics( const FV *fv)
    {
        vtkSmartPointer<vtkFloatArray> arr = this->mapMetrics(fv);
        arr->SetName( label().toStdString().c_str());
        if ( _mapsPolys)
            r3dvis::getPolyData(fv->actor())->GetCellData()->AddArray( arr);
        else
            r3dvis::getPolyData(fv->actor())->GetPointData()->AddArray( arr);
    }   // end _obtainMetrics
};  // end class

}}   // end namespace

#endif

