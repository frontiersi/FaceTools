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

#ifndef FACE_TOOLS_METRIC_METRIC_VALUE_H
#define FACE_TOOLS_METRIC_METRIC_VALUE_H

/*
 * A container for N-dimensional metrics with associated z-scores.
 */

#ifdef _WIN32
#pragma warning( disable : 4251)
#endif

#include <FaceTypes.h>
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricValue
{
public:
    MetricValue(){}
    MetricValue( int id);
    MetricValue( const MetricValue&) = default;
    MetricValue& operator=( const MetricValue&) = default;

    int id() const { return _id;}
    size_t ndims() const { return _values.size();}

    // Overwrite with new values.
    void setValues( const std::vector<double>& dvals) { _values = dvals;}

    // Return the raw value at dimension i.
    double value( size_t i=0) const { return _values.at(i);}

    // Returns the mean at the given age for dimension i.
    double mean( double age, size_t i=0) const;

    // Returns the z-score at the given age for dimension i.
    double zscore( double age, size_t i=0) const;

    void write( PTree&, double age) const;

private:
    int _id;
    std::vector<double> _values;
};  // end class

}}   // end namespace

#endif
