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

#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_H

/**
 * Understands how to perform and record a generic metric calculation.
 */

#include "MetricCalculatorType.h"
#include "GrowthData.h"
#include "MetricSet.h"
#include <LndMrk/Landmark.h>

namespace FaceTools { namespace Metric {

// Growth data by sex then ethnic group code.
using GrowthDataSources = std::unordered_set<GrowthData::CPtr>;

class FaceTools_EXPORT MetricCalculator
{
public:
    using Ptr = std::shared_ptr<MetricCalculator>;
    using CPtr = std::shared_ptr<const MetricCalculator>;

    // Load from file. Reads in all statistics and also makes new
    // growth data pairs for single sex / same ethnicity datasets,
    // and then same sex / two different ethnicity groups.
    // On error, return null.
    static Ptr load( const QString& filepath);

    inline int id() const { return _id;}
    inline const QString& name() const { return _name;}
    inline const QString& description() const { return _desc;}
    inline size_t numDecimals() const { return _ndps;}
    inline QString category() const { return _mct->category();}
    inline size_t dims() const { return _dims;}
    inline bool isBilateral() const { return !_lmks1.empty();}
    inline Vis::MetricVisualiser *visualiser() const { return _mct->visualiser();}

    // Whether or not this metric should be visible (true by default).
    inline void setVisible( bool v) { _visible = v && _mct->visualiser() != nullptr;}
    inline bool isVisible() const { return _mct && _mct->visualiser() && _visible;}

    // Sets compatible growth data against the given model's sex and ethnicity. If the given model's
    // sex if male or female, then growth data having that sex AND non-specific sex (i.e. both female
    // and male) are set. In the case of mixed ethnicity, growth data having the mixed ethnicity
    // (if available) and the individual ethnicities are set. In the case that only one ethnicity
    // is available from the subject's maternal and paternal ethnic codes, then just the corresponding
    // growth data are set. All sources are set if parameter is null or there are no compatible data.
    // If parameter is not null the current source is set to the best match given the model's combination
    // of sex and ethnicity. Multiple sources may be found because of different originating research into
    // the same sex/ethnicity combination and the first of these is used. Given the hierarchical nature
    // of ethnicity, the most specific ethnic match is set that is also a match for the given sex. Since
    // sex is the bigger factor, only compatible sex stats are checked (i.e. only the model's single sex,
    // or mixed sex stats).
    void setCompatibleSources( const FM *fm=nullptr);

    // Returns the set of currently compatible sources. The set of compatible sources can be changed
    // by calling setCompatibleSources above.
    const GrowthDataSources& compatibleSources() const { return _compatible;}

    // Returns all growth data for the given sex/ethnicity combination. If sex and ethnicity are
    // left as their default values then all GrowthDataSources are returned. Note that if a single
    // sex is given then only data for that single sex are returned - not data for both sexes.
    // By default, only data for the exact ethnicity specified are returned (unless ethnicity = 0)
    // If exactEth is false, the ethnicity will be matched against all sources having a parent ethnic
    // group (or being for the same group).
    GrowthDataSources matchingGrowthData( int8_t sex=UNKNOWN_SEX, int ethnicity=0, bool exactEth=true) const;

    // Return the growth data for the given sex, ethnicity and source.
    // Source must be specified if there exists more than one viable set of
    // growth curve data, otherwise the first one is returned.
    GrowthData::CPtr growthData( int8_t sex, int ethnicity, const QString& src="") const;

    // Set the current growth data for this metric.
    void setCurrentGrowthData( GrowthData::CPtr gd) { _cgd = gd;}

    // Returns the currently set growth data for this metric.
    GrowthData::CPtr currentGrowthData() const { return _cgd;}

    // Returns true iff the landmark data exist on the given model to allow measurement.
    bool canMeasure( const FM* fm) const;

    // Record the measurement for this metric and store in the given model's
    // metric values. Returns true iff the measurement could be taken.
    // Overwrites metric values so statistics will need updating.
    bool measure( FM*) const;

    // Return the calculated value for the given face lateral at the given
    // dimension for the given model without updating its metric values.
    // Set pplane to true if the measurement should be taken projected into
    // the 2D plane orthogonal to the model's face normal.
    double calculate( const FM*, FaceLateral, bool pplane=false, size_t dim=0) const;

    ~MetricCalculator();    // Public for Lua

    static QString CUSTOM_STATS_REF;

private:
    MCT* _mct;
    bool _visible;
    int _id;
    size_t _ndps, _dims;
    QString _name, _desc;
    Landmark::LmkList _lmks0, _lmks1;

    // GrowthData keyed by sex-->ethnicity.
    std::unordered_map<int8_t, std::unordered_map<int, GrowthDataSources> > _gdata;
    GrowthData::CPtr _cgd;  // Current (default) for this metric
    GrowthDataSources _compatible;

    void _addGrowthData( GrowthData::Ptr);
    void _combineGrowthDataSexes();
    void _combineGrowthDataEthnicities();
    MetricValue _measure( const FM*, int, const Landmark::LmkList&) const;
    void _findMatchingGrowthData( int8_t, int, bool, GrowthDataSources&) const;
    MetricCalculator();
    MetricCalculator( const MetricCalculator&) = delete;
    void operator=( const MetricCalculator&) = delete;

    GrowthDataSources _getCompatibleGrowthData( const FM*) const;
    GrowthData::CPtr _mostCompatible( int8_t, int, int) const;
};  // end class

}}   // end namespaces

#endif
