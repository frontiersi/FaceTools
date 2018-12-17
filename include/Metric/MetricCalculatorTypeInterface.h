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

/**
 * Represents an N dimensional metric of some sort of calculation over the face.
 * The simplest kind deriving from this is InterlandmarkMetricCalculatorType
 * which is metric of single dimension (a scalar).
 */

#include <FaceTypes.h>
#include <PluginInterface.h>            // QTools
#include <MetricVisualiser.h>           // FaceTools::Vis
#include <RangedScalarDistribution.h>   // rlib

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricCalculatorTypeInterface : public QTools::PluginInterface
{ Q_OBJECT
public:
    virtual int id() const = 0;
    virtual const QString& name() const = 0;            // The individual name of this metric calculator.
    virtual const QString& description() const = 0;     // Description of this metric calculation.
    virtual size_t numDecimals() const = 0;             // The nunber of decimals.

    virtual void setId( int) = 0;
    virtual void setName( const QString&) = 0;          // Set the name of the particular instance.
    virtual void setDescription( const QString&) = 0;   // Set the description.
    virtual void setNumDecimals( size_t) = 0;           // Set the number of decimals to display.

    virtual QString category() const = 0;               // The category (type) name of this metric calculator.
    virtual QString params() const = 0;                 // Parameters specifying how this measurement is made.
    virtual size_t dims() const = 0;                    // Number of dimensions this metric has (defined by category).
    virtual bool isBilateral() const = 0;               // Is this metic bilateral, or medial/singular only?

    virtual Vis::MetricVisualiser* visualiser() = 0;    // Return the visualisation method.
    virtual bool canCalculate( const FM*) const = 0;     // Can this metric be calculated for given model?

    // Measure for d=dimension-1 against given model for the given face lateral.
    virtual double measure( size_t d, const FM*, FaceLateral fl=FACE_LATERAL_MEDIAL) const = 0;

    // Make and return a default initialised version of this object configured with given params.
    using Ptr = std::shared_ptr<MetricCalculatorTypeInterface>;
    virtual Ptr fromParams( const QString&) const = 0;
};  // end class

using MCTI = MetricCalculatorTypeInterface;

}}   // end namespaces

#endif
