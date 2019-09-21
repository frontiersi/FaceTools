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

#include <Metric/MetricCalculatorTypeRegistry.h>
#include <LndMrk/LandmarksManager.h>
#include <iostream>
#include <cassert>
using FaceTools::Metric::MetricCalculatorTypeRegistry;
using FaceTools::Metric::MCT;
using FaceTools::Metric::MC;
using FaceTools::Landmark::LmkList;

std::shared_ptr<MetricCalculatorTypeRegistry> MetricCalculatorTypeRegistry::s_singleton;

// static
std::shared_ptr<MetricCalculatorTypeRegistry> MetricCalculatorTypeRegistry::me()
{
    if ( !s_singleton)
        s_singleton = std::shared_ptr<MetricCalculatorTypeRegistry>( new MetricCalculatorTypeRegistry, [](MetricCalculatorTypeRegistry* d){ delete d;});
    return s_singleton;
}   // end me


// static
void MetricCalculatorTypeRegistry::addMCT( MCT* mct)
{
    QString cat = mct->category().toLower();
    if ( me()->_types.count(cat) > 0)
    {
        std::cerr << "[WARN] FaceTools::Metric::MetricCalculatorTypeRegistry::addMCT: Overwriting existing!" << std::endl;
        delete me()->_types.at(cat);
    }   // end if
    me()->_types[cat] = mct;
}   // end addMCT


// private
MetricCalculatorTypeRegistry::~MetricCalculatorTypeRegistry()
{
    for ( auto p : _types)
        delete p.second;
    _types.clear();
}   // end dtor


MCT* MetricCalculatorTypeRegistry::makeMCT( const QString& acat, int id,
                                            const LmkList* lmks0, const LmkList* lmks1)
{
    static const std::string werr = "[WARN] FaceTools::Metric::MetricCalculatorTypeRegistry::makeMCT: ";
    if ( LDMKS_MAN::count() == 0)
    {
        std::cerr << werr << "Load some landmarks first!" << std::endl;
        return nullptr;
    }   // end if

    const QString cat = acat.toLower();
    if ( me()->_types.count(cat) == 0)
    {
        std::cerr << werr << "Required category '" << cat.toStdString() << "' not present!" << std::endl;
        return nullptr;
    }   // end if

    return me()->_types.at(cat)->make( id, lmks0, lmks1);
}   // end makeMCT
