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

#include <Phenotype.h>
#include <FaceModel.h>
#include <MetricCalculatorManager.h>
using FaceTools::Metric::Phenotype;
using FaceTools::Metric::MetricSet;
using FaceTools::Metric::MetricValue;
using FaceTools::Metric::GrowthData;
using MCM = FaceTools::Metric::MetricCalculatorManager;
using FaceTools::FM;


// private
Phenotype::Phenotype() : _id(-1)
{
    // Register MetricSet for Phenotype determination function:
    _lua.new_usertype<MetricSet>( "MetricSet",
                                  "get", &MetricSet::get);

    _lua.new_usertype<MetricValue>( "MetricValue",
                                    "ndims", &MetricValue::ndims,
                                    "value", &MetricValue::value,
                                    "zscore", &MetricValue::zscore,
                                    "mean", &MetricValue::mean,
                                    "stdv", &MetricValue::stdv);
}   // end ctor


Phenotype::~Phenotype() { }   // end dtor


// public static
Phenotype::Ptr Phenotype::create() { return Ptr( new Phenotype, [](Phenotype *d){ delete d;});}


// public static
Phenotype::Ptr Phenotype::load( const QString& fpath)
{
    bool loadedOk = false;
    Ptr hpo = create();
    hpo->_lua.open_libraries( sol::lib::base);
    try
    {
        hpo->_lua.script_file( fpath.toStdString());
        loadedOk = true;
    }   // end try
    catch ( const sol::error& e)
    {
        std::cerr << "[WARN] FaceTools::Metric::Phenotype::load: Unable to load and execute file '" << fpath.toStdString() << "'!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
    }   // end catch

    if ( !loadedOk)
        return nullptr;

    auto table = hpo->_lua["hpo"];
    if ( !table.valid())
    {
        std::cerr << "[WARN] FaceTools::Metric::Phenotype::load: Missing table 'hpo'!" << std::endl;
        return nullptr;
    }   // end if

    hpo->_id = table["id"].get_or(-1);
    if ( hpo->_id < 0)
    {
        std::cerr << "[WARN] FaceTools::Metric::Phenotype::load: Invalid ID!" << std::endl;
        return nullptr;
    }   // end if

    if ( sol::optional<std::string> v = table["name"]) hpo->_name = v.value().c_str();
    if ( sol::optional<std::string> v = table["region"]) hpo->_region = v.value().c_str();
    if ( sol::optional<std::string> v = table["criteria"]) hpo->_criteria = v.value().c_str();
    if ( sol::optional<std::string> v = table["remarks"]) hpo->_remarks = v.value().c_str();

    if ( sol::optional<sol::table> v = table["synonyms"])
    {
        sol::table synonyms = v.value();
        for ( size_t i = 1; i <= synonyms.size(); ++i)
            hpo->_synonyms.append( synonyms[i].get_or_create<std::string>().c_str());
    }   // end if

    if ( sol::optional<sol::table> v = table["metrics"])
    {
        sol::table metrics = v.value();
        for ( size_t i = 1; i <= metrics.size(); ++i)
            hpo->_metrics.insert( metrics[i].get_or(-1));
    }   // end if

    if ( sol::optional<sol::function> v = table["determine"])
        hpo->_determine = v.value();

    return hpo;
}   // end load


bool Phenotype::isPresent( const MetricSet* mlat, const MetricSet* llat, const MetricSet* rlat) const
{
    if ( !_determine.valid())
        return false;

    // Only evaluate if all the metrics are available for the phenotype
    for ( int mid : _metrics)
    {
        if ( MCM::metric(mid) && MCM::metric(mid)->isBilateral())
        {
            if ( !llat->get(mid) || !rlat->get(mid))
                return false;
        }   // end if
        else
        {
            if ( !mlat->get(mid))
                return false;
        }   // end else
    }   // end for

    bool v = false;
    try
    {
        sol::function_result result = _determine( mlat, llat, rlat);
        if ( result.valid())
            v = result;
        else
            std::cerr << "[WARN] FaceTools::Metric::Phenotype::isPresent: Invalid result from function!" << std::endl;
    }   // end try
    catch (const sol::error& e)
    {
        std::cerr << "[WARN] FaceTools::Metric::Phenotype::isPresent: Error in determination script!" << std::endl;
        std::cerr << "\t" << e.what() << std::endl;
    }   // end catch

    std::cerr << "Phenotype check: " << name().toStdString() << " (" << id() << ") " << (v ? " [PRESENT]" : "") << std::endl;

    return v;
}   // end isPresent


bool Phenotype::isDemographicMatch( const FM* fm) const
{
    const QString lethn = fm->ethnicity().toLower();
    for ( int mid : _metrics)
    {
        const GrowthData* gd = MCM::metric(mid)->growthData(fm);
        if ( gd->ethnicity().toLower() != lethn)
            return false;
    }   // end for
    return true;
}   // end isDemographicMatch
