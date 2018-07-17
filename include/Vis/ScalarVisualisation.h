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

#ifndef FACE_TOOLS_VIS_SCALAR_VISUALISATION_H
#define FACE_TOOLS_VIS_SCALAR_VISUALISATION_H

/**
 * Provides abstract application of scalar colour visualisations to the surface of an actor.
 * Derived types must specify the min and max range values (rangeMin, rangeMax) and implement
 * mapSurfaceActor to map the actual values to do the mapping between the data accessible
 * through FaceControl and the surface actor accessible from its member FaceView. Since
 * scalar visualisations use a per viewer legend, onSelected is overridden to restore the
 * legend key for this visualisation when the corresponding FaceControl is selected.
 */

#include "BaseVisualisation.h"
#include "LegendScalarColourRangeMapper.h"
#include <ColourMappingWidget.h>    // QTools
#include <ObjModelCurvatureMetrics.h>
#include <ChangeEvents.h>
#include <unordered_map>
#include <functional>

namespace FaceTools {
class FaceModel;
class FaceControlSet;
class FaceModelViewer;

typedef std::function<float (const RFeatures::ObjModelCurvatureMetrics*, int)> ScalarMappingFunction;

namespace Vis {

class FaceTools_EXPORT ScalarVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    // Pass in delegate functions to return a floating point value from a polygon.
    ScalarVisualisation( const QString& dname, const QIcon&, const QKeySequence&);
    ScalarVisualisation( const QString& dname, const QIcon&);
    explicit ScalarVisualisation( const QString& dname);
    ~ScalarVisualisation() override;

    // Provide the delegate that maps a scalar to a model polygon.
    void setMappingFunction( const ScalarMappingFunction& mf) { _mfunc = mf;}

    // Returns true if the data to perform mapping are available.
    bool isAvailable( const FaceModel*) const override;

    void apply( const FaceControl*) override;
    void addActors( const FaceControl*) override;
    void removeActors( const FaceControl*) override;

    // Update colour mappings for the given FaceControl from a colour mapping widget.
    // Update only occurs if this visualisation already has a legend mapping for the FaceControl.
    // Returns true iff this visualisation is updated in viewer.
    bool updateColourMapping( const FaceControl* fc, const QTools::ColourMappingWidget*);

    // Update the widget to reflect the colour mappings for the given FaceControl.
    // Update only occurs if this visualisation already has a legend mapping for the FaceControl.
    // Returns true iff the widget was updated.
    bool updateWidget( QTools::ColourMappingWidget*, const FaceControl* fc) const;

protected:
    void addPurgeEvents( Action::ChangeEventSet& ces) const override { ces.insert(Action::SURFACE_DATA_CHANGE);}
    void purge( const FaceControl*) override;

private:
    LegendScalarColourRangeMapper *_lrng;
    ScalarMappingFunction _mfunc;
    std::unordered_map<const FaceControl*, std::pair<float,float> > _mappings;
    std::unordered_set<const FaceControl*> _added;

    // (Re)map this visualisation's scalar data to the FaceControl's surface actor.
    std::pair<float,float> mapActor( const FaceControl*) const;
    void remapColourRange();
};  // end class

}   // end namespace
}   // end namespace

#endif
