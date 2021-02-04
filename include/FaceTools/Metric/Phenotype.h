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

#ifndef FACE_TOOLS_METRIC_PHENOTYPE_H
#define FACE_TOOLS_METRIC_PHENOTYPE_H

#include <FaceTools/FaceTypes.h>
#include "MetricSet.h"
#include <sol.hpp>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT Phenotype
{
public:
    using Ptr = std::shared_ptr<Phenotype>;

    // Load from lua script returning null on error.
    static Ptr load( const QString&);

    // Create a new empty Phenotype object.
    static Ptr create();

    void setId( int id) { _id = id;}
    int id() const { return _id;}

    void setName( const QString& nm) { _name = nm;}
    const QString& name() const { return _name;}

    void setRegion( const QString& r) { _region = r;}
    const QString& region() const { return _region;}

    void setSynonyms( const QStringList& sl) { _synonyms = sl;}
    const QStringList& synonyms() const { return _synonyms;}

    void setObjectiveCriteria( const QString& c) { _ocriteria = c;}
    const QString& objectiveCriteria() const { return _ocriteria;}

    void setSubjectiveCriteria( const QString& c) { _scriteria = c;}
    const QString& subjectiveCriteria() const { return _scriteria;}

    void setRemarks( const QString& r) { _remarks = r;}
    const QString& remarks() const { return _remarks;}

    void setRefs( const QStringList& r) { _refs = r;}
    const QStringList& refs() const { return _refs;}

    // Returns the set of metrics (ids) associated with this HPO term.
    const IntSet& metrics() const { return _metrics;}
    void addMetric( int id) { _metrics.insert(id);}
    void removeMetric( int id) { _metrics.erase(id);}

    // Generate and return the set of metric ids sorted and comma separated.
    QString metricsList() const;

    /**
     * Returns true iff a match of the given model's sex is possible to every metric's
     * statistics necessary to evaluate the presence of this phenotypic indication.
     */
    bool isSexMatch( const FM&) const;

    /**
     * Returns true iff the given model's age is within bounds for all of the metric
     * statistics necessary to evaluate the presence of this phenotypic indication.
     */
    bool isAgeMatch( const FM&) const;

    /**
     * Returns true iff a match of the given ethnicity is possible to every metric's
     * statistics necessary to evaluate the presence of this phenotypic indication.
     */
    bool isMaternalEthnicityMatch( const FM&) const;
    bool isPaternalEthnicityMatch( const FM&) const;

    /**
     * Check if this phenotypic indication is present given the measurements
     * recorded in the metric sets of the provided model and the assessment
     * data (landmarks). Uses the currently set assessment if assessId = -1.
     * Ignores demographic data about the model.
     */
    bool isPresent( const FM&, int assessId=-1) const;

    ~Phenotype(){}  // Public for Lua

private:
    int _id;
    QString _name;
    QString _region;
    QStringList _synonyms;
    QString _ocriteria;
    QString _scriteria;
    QString _remarks;
    QStringList _refs;
    IntSet _metrics;
    sol::state _lua;
    sol::function _determine;

    /**
     * Returns true iff the given model has measurements for all of the
     * corresponding metrics used in the evaluation of the presence of
     * this phenotypic indication.
     */
    bool _hasMeasurements( const FM&, int aid) const;

    bool _isEthnicityMatch( const FM&, int) const;

    Phenotype();
    Phenotype( const Phenotype&) = delete;
    Phenotype& operator=( const Phenotype&) = delete;
};  // end class

}}   // end namespaces

#endif
