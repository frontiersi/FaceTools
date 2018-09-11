/************************************************************************
 * Copyright (C) 2018 Spatial Information Systems Research Limited
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
 */

#include "BaseVisualisation.h"
#include <FaceViewSet.h>

namespace FaceTools {
namespace Vis {

class FaceTools_EXPORT ScalarVisualisation : public BaseVisualisation
{ Q_OBJECT
public:
    // Pass in delegate function ScalarMappingFn that maps float values to
    // the polygon IDs of a FaceModel's underlying RFeatures::ObjModel.
    // Parameters minVal and maxVal are the visualisation's range limits.
    ScalarVisualisation( const QString& dname, const ScalarMappingFn&, float minVal, float maxVal, const QIcon&);

    bool isToggled() const override { return false;}    // Non-toggle so this visualisation is exclusive.

    // Returns true iff this visualisation can be used to map the given data.
    bool isAvailable( const FM*) const override;

    void apply( FV*, const QPoint* mc=nullptr) override;
    void remove( FV*) override;

protected:
    void purge( FV*) override;
    void addPurgeEvents( Action::EventSet& ces) const override { ces.insert(Action::SURFACE_DATA_CHANGE);}
    bool allowShowOnLoad( const FM*) const override { return false;}

private:
    ScalarMapping _scmap;
    ScalarMappingFn _mfunc;
    std::unordered_map<FM*, std::pair<float,float> > _mappings;   // min,max mappings to the data
    FVS _mapped;    // Record the mapped FaceViews
    void remapColourRangeAcrossModels();
};  // end class

}   // end namespace
}   // end namespace

#endif
