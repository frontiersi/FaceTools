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
    virtual std::string category() const = 0;               // The category (type) name of this metric calculator.
    virtual std::string params() const = 0;                 // Parameters specifying how this measurement is made.
    virtual std::string name() const = 0;                   // The individual name of this metric calculator.
    virtual std::string description() const = 0;            // Description of this metric calculation.
    virtual bool isBilateral() const = 0;                   // Is this metic bilateral, or medial/singular only?
    virtual std::string source() const = 0;                 // The source of the metric's data.
    virtual std::string ethnicities() const = 0;            // Ethnicities the calculation is applicable to.
    virtual Sex sex() const = 0;                            // Sex(s) the calculation is applicable to.
    virtual size_t dims() const = 0;                        // Number of dimensions this metric has (defined by category).
    virtual rlib::RSD::Ptr rsd( size_t d=0) const = 0;      // Get the RSD for d=dimension-1
    virtual size_t numDecimals() const = 0;                 // The nunber of decimals.
    virtual Vis::MetricVisualiser* visualiser() = 0;        // Return the visualisation method.

    virtual void setId( int) = 0;
    virtual void setName( const std::string&) = 0;          // Set the name of the particular instance.
    virtual void setDescription( const std::string&) = 0;   // Set the description.
    virtual void setSource( const std::string&) = 0;        // The source of the metric's data.
    virtual void setEthnicities( const std::string&) = 0;   // Ethnicities the calculation is applicable to.
    virtual void setSex( Sex) = 0;                          // Sex(s) the calculation is applicable to.
    virtual void setRSD( size_t d, rlib::RSD::Ptr) = 0;     // Set the RSD for d=dimension-1
    virtual void setNumDecimals( size_t) = 0;               // Set the number of decimals to display.

    virtual bool isAvailable( const FM*) const = 0;         // Can this metric be calculated for given model?

    // Measure for d=dimension-1 against given model for the given face lateral.
    virtual double measure( size_t d, const FM*, FaceLateral fl=FACE_LATERAL_MEDIAL) const = 0;

    // Make and return a default initialised version of this object configured with given params.
    using Ptr = std::shared_ptr<MetricCalculatorTypeInterface>;
    virtual Ptr fromParams( const std::string&) const = 0;
};  // end class

using MCTI = MetricCalculatorTypeInterface;

}}   // end namespaces

#endif
