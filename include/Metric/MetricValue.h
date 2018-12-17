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

#include <FaceTools_Export.h>
#include <boost/property_tree/ptree.hpp>
using PTree = boost::property_tree::ptree;
#include <RangedScalarDistribution.h>
#include <string>

namespace FaceTools { namespace Metric {

struct FaceTools_EXPORT DimensionStat
{
    DimensionStat( double v) : value(v), eage(0), mean(0), stdv(0) {}
    DimensionStat() : value(0), eage(0), mean(0), stdv(0) {}

    double zscore() const { return stdv > 0.0 ? (value - mean) / stdv : 0.0;}

    double value;           // Value
    double eage;            // Evaluation age
    double mean;            // Mean
    double stdv;            // Standard deviation
};  // end struct


class FaceTools_EXPORT MetricValue
{
public:
    MetricValue(){}
    MetricValue( int id);
    MetricValue( int id, const std::vector<DimensionStat>&);
    MetricValue( const MetricValue&);
    MetricValue& operator=( const MetricValue&);

    int id() const { return _id;}

    // Set/get the sex presumed for the statistical data.
    void setSex( int8_t s) { _sex = s;}
    int8_t sex() const { return _sex;}

    // Set/get the ethnicity presumed for the statistical data.
    void setEthnicity( const std::string& e) { _eth = e;}
    const std::string& ethnicity() const { return _eth;}

    // Set/get the source of the statistical data.
    void setSource( const std::string& s) { _src = s;}
    const std::string& source() const { return _src;}

    // Add a new metric statistic only if doing so does not add more dimensions than allowed.
    bool addStat( const DimensionStat& ds);
    size_t ndims() const { return _dstats.size();}

    double value( size_t i=0) const { return _dstats.at(i).value;}
    double zscore( size_t i=0) const { return _dstats.at(i).zscore();}
    double mean( size_t i=0) const { return _dstats.at(i).mean;}
    double stdv( size_t i=0) const { return _dstats.at(i).stdv;}

private:
    int _id;
    int8_t _sex;
    std::string _eth, _src;
    std::vector<DimensionStat> _dstats;
};  // end class


FaceTools_EXPORT PTree& operator<<( PTree&, const MetricValue&);

} }   // end namespace

#endif
