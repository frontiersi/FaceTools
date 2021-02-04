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

#ifndef FACE_TOOLS_METRIC_METRIC_TYPE_REGISTRY_H
#define FACE_TOOLS_METRIC_METRIC_TYPE_REGISTRY_H

#include "MetricType.h"

namespace FaceTools { namespace Metric {

class FaceTools_EXPORT MetricTypeRegistry
{
public:
    static MetricType::Ptr make( const QString& category, const MetricParams&);
};  // end class

}}  // end namespaces

#endif
