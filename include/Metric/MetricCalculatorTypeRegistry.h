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

#ifndef FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_REGISTRY_H
#define FACE_TOOLS_METRIC_METRIC_CALCULATOR_TYPE_REGISTRY_H

#include <MetricCalculatorTypeInterface.h>

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricCalculatorTypeRegistry
{
public:
    static void addMCT( MCT*);
    static MCT* makeMCT( const QString& category, int id, const Landmark::LmkList*, const Landmark::LmkList*);

private:
    static std::shared_ptr<MetricCalculatorTypeRegistry> s_singleton;
    std::unordered_map<QString, MCT*> _types;

    static std::shared_ptr<MetricCalculatorTypeRegistry> me();
    MetricCalculatorTypeRegistry(){}
    ~MetricCalculatorTypeRegistry();
    MetricCalculatorTypeRegistry( const MetricCalculatorTypeRegistry&) = delete;
    void operator=( const MetricCalculatorTypeRegistry&) = delete;
};  // end class

}}  // end namespaces

#endif
