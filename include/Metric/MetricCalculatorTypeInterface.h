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

#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_INTERFACE_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_INTERFACE_H

#include <FaceTypes.h>
#include <PluginInterface.h>            // QTools
#include <MetricVisualiser.h>           // FaceTools::Vis
#include <Landmark.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricCalculatorTypeInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual QString category() const = 0;        // Metric calculator category.

    virtual Vis::MetricVisualiser* visualiser() = 0;    // Return the visualiser

    // Can this metric be calculated for given model assessment (id) and parameters?
    virtual bool canCalculate( const FM*, int aid, const Landmark::LmkList*) const = 0;

    // Measurement(s) against given model and assessment (id) for the given landmark parameters.
    virtual void measure( std::vector<double>&, const FM*, int aid, const Landmark::LmkList*) const = 0;

    // Create and return a clone of this object with the given metric parameters.
    virtual MetricCalculatorTypeInterface* make( int id, const Landmark::LmkList*, const Landmark::LmkList*) const = 0;
};  // end class

using MCT = MetricCalculatorTypeInterface;

}}   // end namespaces

#endif
