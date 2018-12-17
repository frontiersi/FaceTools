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

#ifndef FACE_TOOLS_METRIC_GROWTH_DATA_H
#define FACE_TOOLS_METRIC_GROWTH_DATA_H

#include <FaceTypes.h>
#include <RangedScalarDistribution.h>   // rlib

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT GrowthData
{
public:
    explicit GrowthData( size_t ndims);

    void setEthnicity( const QString& e) { _ethnicity = e;}
    void setSex( int8_t s) { _sex = s;}
    void setSource( const QString& s) { _source = s;}
    void setNote( const QString& n) { _note = n;}
    void setN( int n) {_n = n;}
    void setRSD( size_t d, const rlib::RSD::Ptr& rsd) { _rsds[d] = rsd;}

    const QString& ethnicity() const { return _ethnicity;}
    int8_t sex() const { return _sex;}
    const QString& source() const { return _source;}
    const QString& note() const { return _note;}
    size_t dims() const { return _rsds.size();}
    int n() const { return _n;}
    rlib::RSD::Ptr rsd( size_t d=0) const { return _rsds.at(d);}

private:
    int8_t _sex;
    int _n;
    QString _ethnicity, _source, _note;
    std::vector<rlib::RSD::Ptr> _rsds;
};  // end class

}}   // end namespaces

#endif
