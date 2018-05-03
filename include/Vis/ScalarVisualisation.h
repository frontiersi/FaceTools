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
 * Provides abstract application of scalar colour visualisations to actors.
 * Derived types must specify the min and max range values (rangeMin, rangeMax),
 * and the name of the visualisation should be returned by overridden getDisplayName().
 * Since scalar visualisations use a per viewer legend, setSelected is overridden to
 * restore the legend key for this visualisation when the corresponding FaceControl
 * is selected.
 */

#include "BaseVisualisation.h"
#include <ColourMappingWidget.h>    // QTools
#include <unordered_map>

namespace FaceTools {
namespace Vis {
class LegendScalarColourRangeMapper;

class FaceTools_EXPORT ScalarVisualisation : public SurfaceVisualisation
{ Q_OBJECT
public:
    ScalarVisualisation( const QString& dname, const QIcon&, const QKeySequence&);
    ScalarVisualisation( const QString& dname, const QIcon&);
    explicit ScalarVisualisation( const QString& dname);
    ~ScalarVisualisation() override;

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
    void burn( const FaceControl*) override;

    virtual float rangeMin() const = 0; // The minimum allowed scalar value
    virtual float rangeMax() const = 0; // The maximum allowed scalar value
    virtual void mapSurfaceActor( const FaceControl*) const = 0;    // Map the surface.

private:
    std::unordered_map<const FaceControl*, LegendScalarColourRangeMapper*> _lranges;
};  // end class

}   // end namespace
}   // end namespace

#endif
