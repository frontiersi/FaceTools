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
#include <ColourMappingWidget.h>    // QTools
#include <unordered_map>

namespace FaceTools {
namespace Action {
class ActionMapSurfaceData;
}   // end namespace

namespace Vis {
class LegendScalarColourRangeMapper;

class FaceTools_EXPORT ScalarVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    ScalarVisualisation( const QString& dname, const QIcon&, const QKeySequence&);
    ScalarVisualisation( const QString& dname, const QIcon&);
    explicit ScalarVisualisation( const QString& dname);
    ~ScalarVisualisation() override;

    bool isAvailable( const FaceModel*) const override { return _msd != NULL;}

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
    void onSelected( const FaceControl*) override;
    bool respondCalc() const override { return true;}
    void respondTo( const Action::FaceAction*, const FaceControl*); // Calls mapSurfaceActor
    void purge( const FaceControl*) override;

    virtual void mapSurfaceActor( const Action::ActionMapSurfaceData*, const FaceControl*) = 0;
    virtual float rangeMin() const = 0; // The minimum allowed scalar value
    virtual float rangeMax() const = 0; // The maximum allowed scalar value

private:
    std::unordered_map<const FaceControl*, LegendScalarColourRangeMapper*> _lranges;
    const Action::ActionMapSurfaceData *_msd;
};  // end class

}   // end namespace
}   // end namespace

#endif
