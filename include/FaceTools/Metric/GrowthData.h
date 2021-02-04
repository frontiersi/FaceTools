/************************************************************************
 * Copyright (C) 2021 SIS Research Ltd & Richard Palmer
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

#include <FaceTools/FaceTypes.h>
#include <rlib/RangedScalarDistribution.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT GrowthData
{
public:
    using Ptr = std::shared_ptr<GrowthData>;

    /**
     * Create and return a new GrowthData object from the combination
     * of the given ones. All the GrowthData objects must belong to the
     * same metric (have the same metric ID), and have the same number
     * of dimensions. The sex/ethnicity can be different. In particular,
     * if a combination of ethnicities is created, this is added as a
     * new temporary ethnicity to the static Ethnicities class.
     */
    static Ptr create( const std::vector<const GrowthData*>&);

    // Load growth data statistics from the given file.
    static bool load( const QString&);

    inline int id() const { return _id;}
    void setId( int id) { _id = id;}

    inline int metricId() const { return _mid;}
    inline size_t dims() const { return _rsds.size();}
    inline int8_t sex() const { return _sex;}
    inline int ethnicity() const { return _ethn;}
    inline bool inPlane() const { return _inplane;}

    void setSource( const QString&);
    void addSource( const QString&);
    inline const QString& source() const { return _source;}

    void setNote( const QString&);
    void appendNote( const QString&);
    inline const QString& note() const { return _note;}

    void setLongNote( const QString&);
    inline const QString& longNote() const { return _lnote;}

    void setN( int n) {_n = n;}
    inline int n() const { return _n;}

    void setRSD( size_t d, const rlib::RSD::Ptr& rsd) { _rsds[d] = rsd;}
    inline rlib::RSD::CPtr rsd( size_t d=0) const { return _rsds.at(d);}

    // Returns true iff the given age is >= min and <= max age domain across
    // all of the dimensions of the statistics.
    bool isWithinAgeRange( float age) const;

    ~GrowthData();  // Public for Lua

private:
    int _id;
    const int _mid;
    const int8_t _sex;
    const int _ethn;
    int _n;
    const bool _inplane;
    QString _source, _note, _lnote;
    std::vector<rlib::RSD::Ptr> _rsds;

    static Ptr create( int, size_t, int8_t, int, bool);
    GrowthData( int, size_t, int8_t, int, bool);
    GrowthData( const GrowthData&) = delete;
    void operator=( const GrowthData&) = delete;
};  // end class

}}   // end namespaces

#endif
