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
#include <LandmarksManager.h>
#include <iostream>
#include <cassert>
using FaceTools::Metric::MetricCalculatorTypeRegistry;
using FaceTools::Metric::MCTI;
using FaceTools::Metric::MC;

std::unordered_map<QString, MCTI*> MetricCalculatorTypeRegistry::s_types;


void MetricCalculatorTypeRegistry::addTemplateType( MCTI* mcti)
{
    QString cat = mcti->category().toLower();
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


MCTI::Ptr MetricCalculatorTypeRegistry::createFrom( const QString& acat, const QString& prms)
{
    if ( LDMKS_MAN::count() == 0)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculatorTypeRegistry::createFrom: "
                  << "Load some landmarks first!" << std::endl;
        return nullptr;
    }   // end if

    const QString cat = acat.toLower();
    if ( s_types.count(cat) == 0)
    {
        std::cerr << "[WARNING] FaceTools::Metric::MetricCalculatorTypeRegistry::createFrom: "
                  << "Required category '" << cat.toStdString() << "' not present!" << std::endl;
        return nullptr;
    }   // end if

    MCTI::Ptr mcti = s_types.at(cat)->fromParams(prms);
    assert( mcti != nullptr);
    return mcti;
}   // end createFrom
