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

namespace FaceTools {
namespace Metric {

class FaceTools_EXPORT MetricValue
{
public:
    MetricValue() : _id(-1) {}
    MetricValue( int id, const std::vector<double>& v, const std::vector<double>& z, const std::vector<bool>& zok);

    void setMetricId( int id) { _id = id;}
    int metricId() const { return _id;}

    const std::vector<double>& values() const { return _vals;}
    const std::vector<double>& zscores() const { return _zscs;}
    const std::vector<bool>& zokays() const { return _zoks;}

    // Length of vectors must match and also match the required dimensions
    // for the MetricCalculator associated with the metric id.
    bool set( const std::vector<double>& v, const std::vector<double>& z, const std::vector<bool>& zok);

private:
    int _id;
    std::vector<double> _vals;
    std::vector<double> _zscs;
    std::vector<bool> _zoks;
};  // end class


FaceTools_EXPORT const PTree& operator>>( const PTree&, MetricValue&);
FaceTools_EXPORT PTree& operator<<( PTree&, const MetricValue&);

}   // end namespace
}   // end namespace

#endif
