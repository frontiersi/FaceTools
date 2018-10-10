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

#include <MetricCalculatorTypeRegistry.h>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <cassert>
using FaceTools::Metric::MetricCalculatorTypeRegistry;
using FaceTools::Metric::MCTI;
using FaceTools::Metric::MC;

std::unordered_map<std::string, MCTI*> MetricCalculatorTypeRegistry::s_types;


void MetricCalculatorTypeRegistry::addTemplateType( MCTI* mcti)
{
    std::string cat = mcti->category();
    boost::algorithm::to_lower(cat);
    if ( s_types.count(cat) > 0)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculatorTypeRegistry::addTemplateType: Overwriting existing template!" << std::endl;
        delete s_types.at(cat);
    }   // end if
    s_types[cat] = mcti;
}   // end addTemplateType


void MetricCalculatorTypeRegistry::clearTemplateTypes()
{
    for ( auto p : s_types)
        delete p.second;
    s_types.clear();
}   // end clearTemplateTypes


MCTI::Ptr MetricCalculatorTypeRegistry::createFrom( const std::string& cat, const std::string& prms)
{
    if ( s_types.count(cat) == 0)
        return nullptr;
    MCTI::Ptr mcti = s_types.at(cat)->fromParams(prms);
    return mcti;
}   // end createFrom
