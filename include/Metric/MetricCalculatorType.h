/************************************************************************
 * Copyright (C) 2019 Spatial Information Systems Research Limited
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

#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_H

#include <FaceTypes.h>
#include <MetricVisualiser.h>   // FaceTools::Vis
#include <Landmark.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricCalculatorType
{
public:
    virtual ~MetricCalculatorType();

    virtual QString category() const = 0;        // Metric calculator category.

    virtual Vis::MetricVisualiser* visualiser() = 0;    // Return the visualiser

    // Can this metric be calculated for given model assessment (id) and parameters?
    virtual bool canCalculate( const FM*, int, const Landmark::LmkList*) const { return false;}

    // Measurement(s) against given model and assessment (id) for the given landmark parameters.
    // Set pplane true if the measurement should be taken in the projected 2D plane.
    virtual void measure( std::vector<double>&, const FM*, int aid, const Landmark::LmkList*, bool pplane) const = 0;

    // Create and return a clone of this object with the given metric parameters.
    virtual MetricCalculatorType* make( int id, const Landmark::LmkList*, const Landmark::LmkList*) const = 0;
};  // end class

using MCT = MetricCalculatorType;

}}   // end namespaces

#endif
